# libscv #

A simple C library for parsing and writing CSV files.

## Description ##

This is a tiny C library that can load, manipulate, and save CSV data. 

Examples and source files can be found on 
[github](https://github.com/winobes/libscv).

I feel some obligation to say that there are certainly more robust C 
libraries out there for CSV and other plain text "flat files" (e.g. the one 
found [here](https://github.com/robertpostill/libCSV)  which has a 
customizable parser and malformed data flexibility).

The CSV buffers in this implimentation are very minimal in memory, that is, 
they do not allocate any more memory than they need. This might be a nice 
feature, but it also means frequent calls to realloc when fields are added and
subtracted, so bear that in mind where performance is paramount.

I am a beginner C programmer, and writing and documenting this library is 
primairly an exercise for me. Any feedback is very much appreciated. You can 
contact me at <winobes@gmail.com>.

## Usage ##

This library is designed to be very simple to use. The CSV format is 
not very well standardized, and not all CSV files will be read properly
by the parser, but it should at least be able to handle any file that 
it creates, and any file that adheres to the following rules:

* Rows are separated by a newline character.
* Fields containing any of the following characters are wrapped in text 
deliminators:
  * field deliminator
  * text deliminator
  * newline character
* Any text deliminator in the field is escaped by a preceeding text 
deliminator (note that field deliminators and newline characters should 
not be escaped).

Also take note of the these quirks:

* If a field is set off by text deliminators, but there are characters 
hanging around outside of those text deliminators (but inside the field 
deliminators), they are ignored.
* A field deliminator at the end of a row will cause the parser to 
interpret a trailing blank field (it expects that rows end with just a 
newline character)
* However, newline character at the end of the last line will _not_ 
beinterpreted as an empty trailing row. This is because some UNIX systems 
automatically add a newline character immediately before EOF.

## Installation ##

## TODO ##
* Add some critical functions:
	* csv_set_field()
	* csv_instert_field()
	* csv_get_field_length()
* Write some example programs/function documentation
* Conform non-zero return behavior for adding a row and adding a field
* Create a makefile
* Finish usage and intsallation documentation

## License - WTFPL ##

Copyright (c) 2013 Bill Noble <winobes@gmail.com>

You are free to copy, redistribute, alter, or use source, binaries, 
and documentation associated with this project in any way you see 
fit with or without attribution to the original author.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
