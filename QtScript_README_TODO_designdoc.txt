## a test commit

QtScript interface for Digikam is a work taken up as a Part of 
Season of KDE.

*********************README***************************************
QtScript is a module added to the Qt API from version 4.3 onwards.
It is available to all applications which use Qt internally(KDE applications).

The QtScript is available to the user in the form of a minimalistic Scripting
Console.The user can dynamically send "signals" to digikam and trigger events. 


*********************TO DO****************************************
1.Make necessary changes to CMakeLists.txt file of digikam to enable QtScript.
2.Use the ScriptUI Widget already available in kdelibs
  /trunk/KDE/kdelibs/kjsembed/examples/kjsconsole
  and embed it into the digikam main menu.
3.Expose a very small portion of digikam to QtScript (for testing purposes).
4.Modify the Scripting Console to show available slots the moment the user
  enters <object>. 
  In other words provide dot completion.(Very helpful while prototyping).
5.Extend and expose more of digikam.
6.Test the part exposed (Devise a method to check if the part exposedbehaves)
7.Repeat 4,5 till satisfied

*********************Design DoC************************************
 
