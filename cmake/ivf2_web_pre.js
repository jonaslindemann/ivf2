// ivf2 web pages (ivf2_web_executable, --pre-js): settings from the page URL.
//
// Query parameters named IVF2_* become environment variables of the program before main()
// runs, so the desktop switches work in the browser too:
//   vis_points1.html?IVF2_FIXED_TIMESTEP=0.016&IVF2_RANDOM_SEED=1&IVF2_RENDER_STATS=60
// A parameter without a value ("?IVF2_TRACE_INPUT") is set to 1.
Module['preRun'] = [].concat(Module['preRun'] || [], () => {
  if (typeof location == 'undefined')
    return;
  for (const [name, value] of new URLSearchParams(location.search))
    if (name.startsWith('IVF2_'))
      ENV[name] = value === '' ? '1' : value;
});
