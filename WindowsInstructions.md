# Windows Step-By-Step Example:
## Initial Setup for Mexing
1. Download MS Visual Studio (NOT VS Code).  The community edition is free and includes a Visual C++ compiler that will be easier to work with than MinGW, Cygwin, or other setups that use the GNU compilers.
2. After the download is complete, begin installation.  Select the "Desktop Development" toolset.
3. Download the boost library [here](https://www.boost.org/users/history/version_1_81_0.html).  Unzip the file contents into a directory of your choosing. This library contains the integrators we will use later on.

## Building Mex files in Visual Studio
This tutorial shows how to configure Visual Studio to generate mex functions, without having to invoke the `mex` command in MATLAB. This can be useful if you are plannning to develop more complicated MEX functions and thus have more compilicated builds as a result. Note that most documentation on the internet is geared towards using Visual Studio to mex functions using the C-API, which is incompatible with the C++ API.

### Setting Up a Property Sheet
To avoid having to setup Visual Studio to compile your MEX functions for each new project, it is useful to define a property sheet that can be added to any subsequent projects to make development of new code easier.  Property sheets overwrite any properties that have not been **explicitly** set for the current project.

**NOTE**: Throughout this tutorial, MATLAB_ROOT refers to the root directory where MATLAB is installed.  For most people, this is something like `C:/Program Files/MATLAB/2022b`

1. Open a new project in Visual Studio.
2. Click View > Other Windows > Property Manager. You will see your current project shown in the Property Manager window.
3. Right click on the name of your project and click "Add New Project Property Sheet."  Title your property sheet and save (e.g. MexSetup.props) and save.
4. Open the Subfolder Release | x64 (or any of the others).  You should see your property sheet name. Double click on your property sheet.
5. Under Common Properties > General, change Target Extension to .mexw64 (or .mexw32 if using a 32 bit system).
6. Under C/C++ > General, navigate to "Additional Include Directories."  Add MATLAB_ROOT/extern/include, BOOST_ROOT and mex-integration/src/include.  BOOST_ROOT is the root directory for where you unzipped your boost download, e.g. C:/Program Files/boost_1_81_0.  This tells Visual Studio where to look for the necessary C++ and C header files for mexing, which are necessary for building your solution and provides helpful syntax highlighting and suggestions.
![IncludeSetup](/TutorialFigures/IncludeSetup.PNG?raw=true)
7. Under Linker > General, select Additional Library Directories and add MATLAB_ROOT/extern/lib/win64/microsoft.  This specifies the location of the binary library files needed for mexing.
![LibSetup](/TutorialFigures/LibSetup.PNG?raw=true)
8. Under Linker > Input > Additional Dependencies, add the following library files:

        1. libMatlabCppSharedLib.lib

        2. libmex.lib

        3. libMatlabDataArray.lib

        4. libmat.lib (Optional)

        5. libMatlabEngine.lib (Optional)

        6. libmx.lib (Optional)
9. Save these changes.  Your properties sheet can now be added to any new project via the Property Manager (Add Existing Property Sheet).

## Test Property Sheet Setup
1. Open a new source file in the current project.
2. Create a new MexFunction, something similar to below:
```
    #include "mex.hpp"
    #include "mexAdapter.hpp"

    using matlab::mex::ArgumentList;
    using namespace matlab::data;

    class MexFunction : public matlab::mex::Function
    {
        void operator()(ArgumentList outputs, ArgumentList inputs)
        {
            ArrayFactory af;
            CharArray hello_arr = af.createCharArray("Hello Matlab!");
            outputs[0] = hello_arr;
        }
    };
```
3. Right click on your current project in the Solution Explorer and open the Properties tab.  Under Configuration Properties > General, change the configuration type to .dll (or .lib).  This prevents VS from trying to make an executable program when there is no defined `main` function.  
![dll_setup](/TutorialFigures/DllSetup.PNG)
4. Make sure that you have applied your property sheet to this project!  When you have done so, ensure you are set your build configuration to "Release."  Then press `CTRL+Alt+F7` to build your solution. 
![release_mode](/TutorialFigures/ReleaseMode.PNG?raw=true)
5. If build is successful, you should should see in your project root directory a new folder (x64), with a subdirectory called "Release." In this directory, you will find your compiled MEX function.  
6. Navigate to this directory and call this function from MATLAB to ensure that the build was successful.
