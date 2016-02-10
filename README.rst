dstat
-----
A lot of other status monitors for `dwm <http://dwm.suckless.org/>`_ attempt to have an extensible plugin style.
There is nothing wrong with doing so, and it does allow for greater simplicity for new users.
However, through adding such functionality, also added is an extra level of complexity.

This is a very small status program (~26KiB when built normally) that aims to be functional and flexible but primarily small and fast.
Each module is fairly self-explanatory, and the defaults are meant to be as sane as possible.

To-Do
=====
#. Link to ``libpulse`` and fill out ``get_aud_volume()`` and ``get_aud_mute()``
#. Add a ``configure`` script to allow for pre-compile-time customization of the default values without actually requiring source modification
#. Add support for symbolic representation rather than letter prefixes (decreases size of buffer while increasing readbility)
#. Add support for color escapes (not sure if this is a good idea)

