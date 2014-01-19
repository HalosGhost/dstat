dstat
-----
A lot of other status monitors for `dwm <http://dwm.suckless.org/>`_ attempt to have an extensible plugin style.
There is nothing wrong with doing so, and it does allow for greater simplicity for new users.
However, through adding such functionality, also added is an extra level of complexity.

This is an incredibly small status program (7.1 KiB when built normally) that aims to be as simple as possible.
Each monitor is fairly self-explanatory, though the setup is currently specific to my hardware.

To-Do
=====
#. Simplify the buffer passing between monitors through ``snprintf()`` (will increase stability in the case of a particular monitor failing)
#. Remove dependency on ``ponymix`` in favor of using ``pactl`` and/or ``pacmd`` (will remove a run-time dependency and make use more generalizable)
#. Add support for symbolic representation rather than letter prefixes (decreases size of buffer while increasing readbility)
#. Add support for color escapes (not sure if this is a good idea)
