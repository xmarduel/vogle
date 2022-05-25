# vogle graphic library
In the beginnings of 2000 (yes), I used to work with the vogle graphic library **version 2.0b**.
The main improvement over the previous verison (**version 1.3**) was the support for multiple windows.

The **vopl** plotting lib (based on **vogle**) had not yet reached the 2.0 level, so I did it at the time.
I added many things in **vopl** (ex: subpanel handling ala pgplot, countour on ucd mesh etc), so I named
this **version 2.0b** too, and the diff between my version and the official last version is quite large.

Note that the **python wrappers** had only been made for MACOS and python-2.7 (when I updated my source some years ago).
But it should be easy to update with a newer version of python and swig. The related makefiles must be updated.

The **vogle** and **vopl** libraries are not built nor there are makefiles because all of this used to be bundled inside a XCode project
