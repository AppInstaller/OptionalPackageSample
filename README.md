# OptionalPackageSample
This is a example solution that shows how content can be componentized across optional packages. It contains the following projects
1. MyMainApp
2. OptionalPackage
3. ActivatableOptionalPackage1
4. ActivatableOptionalPackage2

The MyMainApp is the main project that loads content form its optional packages. The "OptionalPackage" project is a content only optional package that contains a text file. This does not have an entry in applist as this is merely content that is loaded by the MyMainApp. The "ActivatableOptionalPackage" projects are related set optional packages that contain a native dll, a text file and also has an app list entry which demonstrates that these could be apps that a user can launch from start. 

Read more about optional packages here - https://blogs.msdn.microsoft.com/appinstaller/2017/04/05/uwpoptionalpackages/

How to deploy?
Via the Visual Studio Solution Explorer

1. Right click MyMainApp project and deploy it. This will also deploy ActivatableOptionalPackage1 & ActivatableOptionalPackage2
2. Right click & Deploy the OptionalPackage
3. With the MyMainApp project set as the startup project (defauly) press F5 or hit the Debug button. 
Note: If you are prompted to build the MyMainApp project, hit No.
4. When the app is launched, press "Load Content from Optional Packages" and the output window will show the packages that were enumerated and also display the content within the optional packages.
