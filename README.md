# whats_stat
A CLI tool to analyze Whatsapp char logs.

## Installation 
Just clone the repository and then compile `what_stat.c` using gcc 

## Usage 
   what_stat FILENAME [PARAMS] [OPTIONS] <br />
   FILENAME is the path to the file you can obtain by clicking the "Export chat" button in the whatsapp app.

## Parameters
  -m { int }	Set a maximum word limit. Default: Infinite <br />
  -u { int }	Set a maximum user limit. Default: 256 <br />
  -i {float}	Set Interval between two conversations in hours. Default: 1.0h

##Options
  -e {char}	Search For a specific word <br />
  -g {none}	Show global results too (if combined with `-e`)
