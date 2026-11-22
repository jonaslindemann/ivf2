# Security Policy

## Supported versions

ivf2 is developed in spare time. Security fixes are made on the `main` branch and included in
the next release; older releases are not patched.

| Version                       | Supported |
|-------------------------------|-----------|
| `main` and the latest release | Yes       |
| Older releases                | No        |

## Reporting a vulnerability

**Please do not report security vulnerabilities in public issues, discussions or pull requests.**

Report them privately through GitHub:

1. Go to the [Security tab](https://github.com/jonaslindemann/ivf2/security) of the repository.
2. Click **Report a vulnerability**
   ([direct link](https://github.com/jonaslindemann/ivf2/security/advisories/new)).
3. Describe the problem, the affected versions or commits, and how to reproduce it (for example
   a file that crashes a loader). A suggested fix is welcome but not required.

Only you and the maintainers can see the report. GitHub's guide to
[privately reporting a security vulnerability](https://docs.github.com/en/code-security/security-advisories/guidance-on-reporting-and-writing-information-about-vulnerabilities/privately-reporting-a-security-vulnerability)
explains the process.

## What to expect

- An acknowledgement within 14 days (best effort).
- Discussion of the report and the fix in the private advisory; you may be invited to a
  temporary private fork to work on the fix together.
- When a fix is released, the advisory is published with credit to you, unless you prefer to
  stay anonymous. A CVE can be requested through GitHub when appropriate.

## Scope

ivf2 is a library that runs inside your application. Most relevant are problems where untrusted
input causes memory corruption or code execution, for example in loading models, images, fonts,
audio, shaders, scene files, Renoise songs or JSON configuration. Bugs in third-party
dependencies (assimp, FreeType, stb, miniaudio, ...) should be reported to those projects; tell
us as well if ivf2 needs to update or work around them.
