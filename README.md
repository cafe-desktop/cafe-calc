# Introduction.

This is `cafe-calc`, a fork of `gnome-calc`, the calculator application
that was previously in the OpenWindows Deskset of the Solaris 8
operating system.

It incorporates a multiple precision arithmetic packages based on the
work of Professor Richard Brent, who has also kindly given me
permission to make it available.

There is a single graphics driver for Ctk3 included with this release.

## Getting started.

The manual pages also describe how to use `cafe-calc` in detail.


## Calctool history.

Calctool was a project I worked on before I joined the OpenWindows
DeskSet engineering group at Sun. It was originally released to
comp.sources.unix in the late 1980's, and worked with many different
graphics packages including SunView, X11, Xview, NeWS and MGR. There
was also a version that worked on dumb tty terminals.

It used a double-precision maths library that was a combination of the
work of Fred Fish and various routines that were in the BSD 4.3 maths
library.

A lot of people in the community provided feedback in the form of
comments, bug reports and fixes. In 1990, I started working in the
DeskSet engineering group. I was working for Sun Microsystems in
Australia at the time, (having moved there from England in 1983).

I searched around looking for multiple precision maths libraries and
found a package called MP written in FORTRAN by Richard Brent. I
converted it to C, adjusted the glue between the resultant code and the
calctool code, and this went on to be the basis of the calculator that
was in the OpenWindows DeskSet. I also added scientific, financial and
logical modes. This calctool was also the basis of the dtcalc
application that is a part of CDE (albeit I had nothing to do with
that).

With its inclusion in the CAFE CVS repository, it was renamed to
`cafe-calc`.

More recently, Sami Pietila provided arithmetic precedence support and
Robert Ancell converted the UI to use Glade.


## Acknowledgements.

See the AUTHORS file.

Suggestions for further improvement would be most welcome, plus bug
reports and comments.

The Cafe Team.