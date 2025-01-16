# JSON Validation Process

## Overview
This document outlines the process of validating JSON inputs using test files from [JSONTestSuite](https://github.com/nst/JSONTestSuite).

## Validation Process
The validation process is handled at different stages of the parsing process:
1. **Tokenization**
2. **Validation**

If an incorrect input is encountered at any step, the process halts, avoiding unnecessary further processing. There should be no errors during the final parsing step.

## Missing Cases
All the error cases encountered during the design and development of the library are correctly handled. However, if you come across an error case that is not covered by the library or if the library fails to parse a valid input, please feel free to open an issue addressing your concern. Help and criticisms are very much welcomed!

## Documentation
You can find valid JSON schemas at:
- [json.org](https://www.json.org/json-en.html)
- [RFC 8259](https://datatracker.ietf.org/doc/html/rfc8259)