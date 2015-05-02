Python 2/3 multiplexing launcher
================================

pythonmux is a binary intended to be installed at `/usr/bin/python` to
multiplex between Python 2 and 3 in a backwards-compatible way. Notably,
it will default to running Python 2 to match the historical interface of
`/usr/bin/python`, but scripts can specify that they can run on either
Python 2 or 3. Such scripts can be run on a system that only has Python
3 installed, if this launcher is present.

For more details, see my blog post, [A proposal for `/usr/bin/python`
between Python 2 and Python
3](https://ldpreload.com/blog/usr-bin-python-23).

pythonmux is free software licensed under the 2-clause BSD license.
