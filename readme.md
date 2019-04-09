# Watermark Remove

This App remove the watermark adding by StarUML on svg generated files

## Demo


## Install

To recompile project, open a bash in the root directory and type :

```bash

mvn clean compile assembly:single

```

You can find the jar file in "target" folder


## Usage

svg files must be placed into the jar folder

Add argument to run the app:

* To choose a specific file

`java -jar watermark-app-jar-with-dependencies.jar file`

* To convert all svg files in the directory :

`java -jar watermark-app-jar-with-dependencies.jar folder`

## Copyright

This project is licensed under the MIT License - see the [license](LICENSE) file for details.