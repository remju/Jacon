![Build Status](https://github.com/julien-remmery-vinci/Jacon/actions/workflows/ci.yml/badge.svg)
# Jacon
Easy to use Json parsing library in C

# Description
Serialize a struct into a string.
Deserializing a Json string into a struct, if input is a valid object, also get a Map of values for an easier querrying. 

# Tests
Inputs tested using validation files from:
- [JSONTestSuite](https://github.com/nst/JSONTestSuite)

# Origin of Jacon
- Name :
Json -> Jason, Jason is already used in other langages for parsing Json.
We doing C so replace 's' by 'c' -> Jacon.
As easy as that.

- Why : I want to experience building a json parser, I plan to use it in my C webserver implementation.

- Why tho : Building things from sratch is fun.

# Thanks
- Thanks to - [JSONTestSuite](https://github.com/nst/JSONTestSuite) for the test files.