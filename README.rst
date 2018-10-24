dstat
-----

|coverity|

.. |coverity| image:: https://img.shields.io/coverity/scan/7913.svg
   :alt:              Coverity Scan Build Status
   :target:           https://scan.coverity.com/projects/halosghost-dstat

A lot of other status monitors for `dwm <http://dwm.suckless.org/>`_ attempt to have an extensible plugin style.
There is nothing wrong with doing so, and it does allow for greater simplicity for new users.
However, through adding such functionality, also added is an extra level of complexity.

In addition, this status monitor allows writing to ``stdout``, so it will work with any status bar that can read from ``stdin``.

This is a very small status program (~24KiB when built normally and stripped) that aims to be functional and flexible but primarily small and fast.
Each module is fairly self-explanatory, and the defaults are meant to be as sane as possible.
