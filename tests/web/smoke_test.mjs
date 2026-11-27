// Smoke test for the web build (preset wasm): opens every page in headless Chrome and checks
// that it starts, keeps rendering frames and logs no errors.
//
//   node tests/web/smoke_test.mjs <bin dir> [options] [page ...]
//
//   <bin dir>            build/wasm/bin (the .html/.js/.wasm/.data files)
//   page ...             page names without .html (default: every page in <bin dir>)
//   --screenshots <dir>  save a JPEG of each page as <dir>/<name>.jpg
//   --jobs <n>           pages tested at the same time (default 2)
//   --time <ms>          time each page runs after loading (default 3000)
//   --size <w>x<h>       browser window size (default 320x180: SwiftShader's cost is mostly per pixel)
//
// A page fails if it shows an error, throws, logs an ERROR or WARN line (ivf::Logger) or a
// WebGL error, or renders fewer than two frames after loading (the main loop runs). shader_check must report "0 failed".
// Needs Node 22+ (the emsdk's node is fine) and Chrome or Chromium: CHROME or CHROME_BIN
// names the executable, otherwise the usual install locations are tried. WebGL runs on
// SwiftShader, so no GPU is needed. Exits with 1 if a page fails.
//
// SMOKE_ANGLE=d3d11 (or gl, metal, ...) uses the GPU instead, e.g. to time shader compilation
// as browsers on that platform see it (the driver may add compiler warnings, which then fail
// the page); SMOKE_DEBUG=1 prints the frame count while a page runs.
import http from 'node:http';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawn } from 'node:child_process';

const args = process.argv.slice(2);
const options = { screenshots: null, jobs: 2, time: 3000, size: '320x180' };
const pages = [];
let binDir = null;
for (let i = 0; i < args.length; i++) {
    const a = args[i];
    if (a.startsWith('--')) {
        const key = a.slice(2);
        if (!(key in options)) {
            console.error(`unknown option ${a}`);
            process.exit(2);
        }
        options[key] = args[++i];
    } else if (binDir === null)
        binDir = a;
    else
        pages.push(a.replace(/\.html$/, ''));
}
if (!binDir) {
    console.error('usage: node smoke_test.mjs <bin dir> [--screenshots dir] [--jobs n] [--time ms] [page ...]');
    process.exit(2);
}
if (pages.length === 0)
    for (const f of fs.readdirSync(binDir).sort())
        if (f.endsWith('.html') && f !== 'index.html')
            pages.push(f.slice(0, -5));
const missing = pages.filter((p) => !fs.existsSync(path.join(binDir, p + '.html')));
if (missing.length) {
    console.error(`no such page in ${binDir}: ${missing.join(', ')}`);
    process.exit(2);
}
const [width, height] = options.size.split('x').map(Number);
const runTime = Number(options.time);
const loadTimeout = 90000; // downloading and compiling a page can be slow on CI runners
const frameTimeout = 60000;
const minFrames = 2; // heavy pages draw a frame every ~30 s with SwiftShader on CI runners
if (options.screenshots)
    fs.mkdirSync(options.screenshots, { recursive: true });

function findChrome() {
    for (const c of [process.env.CHROME, process.env.CHROME_BIN])
        if (c)
            return c;
    const candidates = {
        win32: ['C:/Program Files/Google/Chrome/Application/chrome.exe',
                'C:/Program Files (x86)/Google/Chrome/Application/chrome.exe'],
        darwin: ['/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',
                 '/Applications/Chromium.app/Contents/MacOS/Chromium'],
    }[process.platform] ?? ['/usr/bin/google-chrome', '/usr/bin/google-chrome-stable', '/usr/bin/chromium',
                            '/usr/bin/chromium-browser'];
    const found = candidates.find((c) => fs.existsSync(c));
    if (!found) {
        console.error('Chrome not found: set CHROME to its executable');
        process.exit(2);
    }
    return found;
}
const chromePath = findChrome();

// Static file server for the pages
const types = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm' };
const server = http.createServer((req, res) => {
    const file = path.join(binDir, decodeURIComponent(new URL(req.url, 'http://x').pathname));
    fs.readFile(file, (err, data) => {
        if (err) {
            res.writeHead(req.url === '/favicon.ico' ? 204 : 404);
            res.end();
            return;
        }
        res.writeHead(200, { 'Content-Type': types[path.extname(file)] ?? 'application/octet-stream' });
        res.end(data);
    });
});
await new Promise((r) => server.listen(0, '127.0.0.1', r));
const baseUrl = `http://127.0.0.1:${server.address().port}`;

// Lines that make a page fail, and harmless ones that match them
const badLine = /^\[exception\]|^\[log:error\]|^\d\d:\d\d:\d\d\.\d{3} (ERROR|WARN)\b|WebGL: |GL_INVALID|INVALID_|null function/;
// The blit message comes from Chrome itself (ANGLE below WebGL, seen once while the canvas was
// resized in a page that never blits); a blit of ours would fail WebGL's own validation first
// and log a "WebGL: INVALID_OPERATION" line.
const harmless = /GPU stall due to ReadPixels|favicon\.ico|^\[log:warning\] \[\.WebGL-\w+\]GL_INVALID_OPERATION: glBlitFramebuffer: Blit feedback loop/;
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

// One Chrome per page: pages in background tabs would not get animation frames
async function testPage(name) {
    const profile = fs.mkdtempSync(path.join(os.tmpdir(), 'ivf2-smoke-'));
    const chrome = spawn(chromePath, [
        '--headless=new', '--remote-debugging-port=0', `--user-data-dir=${profile}`,
        `--use-angle=${process.env.SMOKE_ANGLE ?? 'swiftshader'}`, '--enable-unsafe-swiftshader', '--no-first-run',
        '--no-default-browser-check', '--disable-background-timer-throttling', `--window-size=${width},${height}`,
        ...(process.env.CI ? ['--no-sandbox'] : []), 'about:blank']);
    const result = { name, ok: false, frames: 0, seconds: 0, problems: [], log: [] };
    const started = Date.now();
    let ws = null;
    try {
        const browserUrl = await new Promise((resolve, reject) => {
            let buf = '';
            chrome.stderr.on('data', (d) => {
                buf += d;
                const m = buf.match(/DevTools listening on (ws:\/\/\S+)/);
                if (m)
                    resolve(m[1]);
            });
            chrome.on('exit', () => reject(new Error('Chrome exited: ' + buf.trim().split('\n').slice(-3).join(' '))));
            setTimeout(() => reject(new Error('Chrome did not start')), 30000);
        });
        const port = new URL(browserUrl).port;
        const targets = await (await fetch(`http://127.0.0.1:${port}/json/list`)).json();
        ws = new WebSocket(targets.find((t) => t.type === 'page').webSocketDebuggerUrl);
        await new Promise((r, j) => {
            ws.addEventListener('open', r);
            ws.addEventListener('error', j);
        });
        let id = 0;
        const pending = new Map();
        ws.addEventListener('message', (e) => {
            const msg = JSON.parse(e.data);
            if (msg.id && pending.has(msg.id)) {
                pending.get(msg.id)(msg);
                pending.delete(msg.id);
            } else if (msg.method === 'Runtime.consoleAPICalled')
                result.log.push(msg.params.args.map((a) => a.value ?? a.description ?? '').join(' '));
            else if (msg.method === 'Runtime.exceptionThrown') {
                const d = msg.params.exceptionDetails;
                result.log.push('[exception] ' + (d.exception?.description ?? d.text));
            } else if (msg.method === 'Log.entryAdded')
                result.log.push(`[log:${msg.params.entry.level}] ${msg.params.entry.text}`);
        });
        const send = (method, params = {}) => new Promise((r) => {
            const i = ++id;
            pending.set(i, r);
            ws.send(JSON.stringify({ id: i, method, params }));
        });
        const evaluate = async (expression) =>
            (await send('Runtime.evaluate', { expression, returnByValue: true })).result?.result?.value;

        await send('Runtime.enable');
        await send('Log.enable');
        await send('Page.enable');
        await send('Emulation.setDeviceMetricsOverride', { width, height, deviceScaleFactor: 1, mobile: false });
        // Count animation frames (the Emscripten main loop runs on requestAnimationFrame)
        await send('Page.addScriptToEvaluateOnNewDocument', { source: `
            window.__ivf2Frames = 0;
            const raf = window.requestAnimationFrame.bind(window);
            window.requestAnimationFrame = (cb) => raf((t) => { window.__ivf2Frames++; cb(t); });` });
        await send('Page.navigate', { url: `${baseUrl}/${name}.html` });

        // Loaded: the status box is hidden (running) or shows an error
        let state = '';
        for (const t0 = Date.now(); Date.now() - t0 < loadTimeout; await sleep(250)) {
            state = await evaluate(`location.pathname !== "/${name}.html" || document.readyState !== "complete" ? "" :
                                    (document.getElementById("status")?.className ?? "none")`);
            if (state === 'hidden' || state === 'error' || state === 'none')
                break;
        }
        if (state === 'error')
            result.problems.push('page error: ' + await evaluate('document.getElementById("message").textContent'));
        else if (state !== 'hidden' && state !== 'none')
            result.problems.push(`not started after ${loadTimeout / 1000} s`);
        else {
            // Run for a while and count frames
            const f0 = await evaluate('window.__ivf2Frames');
            const t0 = Date.now();
            for (;;) {
                await sleep(250);
                result.frames = (await evaluate('window.__ivf2Frames')) - f0;
                const elapsed = Date.now() - t0;
                if (process.env.SMOKE_DEBUG)
                    console.log(`  ${name} ${elapsed} ms ${result.frames} frames`);
                if ((elapsed >= runTime && result.frames >= minFrames) || elapsed >= frameTimeout)
                    break;
            }
            if (result.frames < minFrames)
                result.problems.push(`only ${result.frames} frames in ${frameTimeout / 1000} s`);
        }
        const title = await evaluate('document.title');
        if (name === 'shader_check' && !/: 0 failed of [1-9]/.test(title))
            result.problems.push(`shader_check: "${title}"`);
        if (options.screenshots) {
            const shot = await send('Page.captureScreenshot', { format: 'jpeg', quality: 80 });
            if (shot.result?.data)
                fs.writeFileSync(path.join(options.screenshots, name + '.jpg'), Buffer.from(shot.result.data, 'base64'));
        }
    } catch (e) {
        result.problems.push(String(e.message ?? e));
    } finally {
        ws?.close();
        chrome.kill();
        await sleep(300);
        try {
            fs.rmSync(profile, { recursive: true, force: true });
        } catch {
            // Chrome may still hold files; the temporary directory is cleaned up later
        }
    }
    const bad = result.log.filter((l) => badLine.test(l) && !harmless.test(l));
    result.problems.push(...bad.slice(0, 10));
    result.ok = result.problems.length === 0;
    result.seconds = (Date.now() - started) / 1000;
    return result;
}

// Run the pages, a few at a time
const results = [];
const queue = [...pages];
await Promise.all(Array.from({ length: Math.max(1, Number(options.jobs)) }, async () => {
    while (queue.length > 0) {
        const r = await testPage(queue.shift());
        results.push(r);
        console.log(`${r.ok ? 'ok  ' : 'FAIL'} ${r.name.padEnd(24)} ${String(r.frames).padStart(4)} frames ` +
                    `${r.seconds.toFixed(1).padStart(5)} s`);
        for (const p of r.problems)
            console.log('       ' + p);
        if (!r.ok && process.env.CI)
            for (const l of r.log.slice(-20))
                console.log('     | ' + l);
    }
}));
server.close();

const failed = results.filter((r) => !r.ok).sort((a, b) => a.name.localeCompare(b.name));
console.log(`\n${results.length - failed.length} of ${results.length} pages passed` +
            (failed.length ? `; failed: ${failed.map((r) => r.name).join(', ')}` : ''));
if (process.env.GITHUB_STEP_SUMMARY) {
    let md = `### Web smoke test\n\n${results.length - failed.length} of ${results.length} pages passed.\n`;
    if (failed.length) {
        md += '\n| Page | Problem |\n| --- | --- |\n';
        for (const r of failed)
            md += `| ${r.name} | ${r.problems[0].replace(/\|/g, '\\|').slice(0, 200)} |\n`;
    }
    fs.appendFileSync(process.env.GITHUB_STEP_SUMMARY, md);
}
process.exit(failed.length ? 1 : 0);
