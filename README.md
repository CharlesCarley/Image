# Image

A simple image drawing utility library

## Building

This project builds with CMake and has two external git repositories as dependencies.

+ [FreeImage](https://github.com/CharlesCarley/FreeImage)
+ [Utils](https://github.com/CharlesCarley/Utils)

The files [gitupdate.py](gitupdate.py) or [gitupdate.bat](gitupdate.bat) help to automate initial cloning and
with keeping the modules up to date.

Once this project has been cloned. The following command will initialize the external modules.

```txt
python gitupdate.py 
...
gitupdate.bat 
```

then building with CMake.
```txt
mkdir Build
cd Build
cmake .. 
```
