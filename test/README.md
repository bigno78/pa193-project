# Dataset for PA193 Project

This archive contains dataset for the semestral project of PA193 course.

The dataset directory contains 50 *.txt files that are input files and the
corresponding 50 *.json files are expected outputs.

To compare the output of your parser with the expected output, you can use
script `output_compare.py`. The script takes as input two files - output of
your parser and the expected output 

```bash
python3 output_compare.py reference.json parser_output.json
```
