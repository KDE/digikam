# HUPnP
Fork of Herqq UPnP (HUPnP), a software library for building UPnP devices and control points conforming to the UPnP Device Architecture version 1.1.

This fork aims the port of HUPnP to Qt5 and continue to improve this great library.

All about the original project can be found here : http://hupnp.linada.fi/

********************

##HUPnP

Herqq UPnP (HUPnP) is a software library for building UPnP devices and control points conforming to the UPnP Device Architecture version 1.1. It is designed to be simple to use and robust in operation. It is built using C++ and the Qt Framework following many of the design principles and programming practices used in the Qt Framework. It integrates into Qt-based software smoothly and enables truly rapid UPnP development.

HUPnP is the result of careful interface design and test-driven development. The interface is simple, yet powerful. It is easy to get started with HUPnP and it enables you to build and interact with any type of UPnP device. If you're interested in trying it out, please download the latest version and check the documentation for more information how to get started.

##Goals

Simplicity & Usability. Above everything else, HUPnP is designed to make almost any type of UPnP development simple. HUPnP is nothing but a comprehensive and documented class library that is built for re-use and extension. Each class is designed for a purpose, which function is encapsulated behind an intuitive interface. Simplicity applied to usability has been leading the HUPnP development and it will continue to do so in the future.

Conformance. All the power and potential of UPnP technology relies on standards and specifications. It is mandatory to follow them to the letter. All too often someone deviates and others follow, which can easily result in interoperability nightmare. HUPnP attempts to be fully standard-compliant. However, if you discover nonstandard behavior in HUPnP, please report it and it will be checked and fixed as soon as possible.

Cross-platform. UPnP is about platform, media and device independence. Similarly, the tools and libraries used to build UPnP software should be at least platform independent. One of the main reasons HUPnP is tightly integrated into the Qt Framework is that Qt is time-proven cross-platform technology that provides solid platform abstraction. In addition, it is one of the leading cross-platform GUI tookits that has been used worldwide in numerous successful software projects.

##HUPnPAv

Herqq UPnP A/V (HUPnPAv) is a software library for building UPnP A/V devices and control points conforming to the UPnP A/V specifications available at the UPnP Forum. It is built directly above the HUPnP core library and as such it follows the same design principles and programming practises and shares the same general goals.

Whereas HUPnP attempts to abstract as much of the technical details of the UPnP Device Architecture as feasible, HUPnPAv attempts to do the same for the UPnP A/V Device Architecture. HUPnPAv offers classes that simplify the development of any type of UPnP A/V development both at the server- and client-side. HUPnPAv API is designed to be progressive, which means that it has classes that give you more control at the expense of ease of use and classes that give you less control while being much simpler to use.

For example, HUPnPAv offers classes that enable you to build any type of UPnP A/V device or service without worrying too much of the details of the UPnP Device Architecture. These classes abstract the UPnP Device Architecture well and give you full control of what the UPnP A/V devices and services should do, but they can be fairly laborous to use. On the other hand, by using some higher level APIs initializing and loading data to an extensible media server component can be done in around 10 lines of code. Or, discovering a MediaServer from the client side and browsing everything its ContentDirectory offers can be done in around 5 lines of code.
