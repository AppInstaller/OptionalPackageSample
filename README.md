# OptionalPackageSample
This is a sample of how you can create an optional package that contains a text file and how you can enumerate and load text from the optional package.

1. Build and deploy the MyMainApp. 
2. Deploy the OptionalPackage, ActivatableOptionalPackage1 & ActivatableOptionalPackage2

OptionalPackage project is a content only optional package. It is an app that does not have an entry in applist. The ActivatableOptionalPackages demonstrates that you can build an optional package that has code in it that can be loaded by MyMainApp. It also has an applist entry which means that users can launch the optional package directly from Start. 
