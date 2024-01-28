# Autocomplete in C

Simple autocompelte tool in C. You will need a words file to use with this
autocompelte tool.

For example you can use `nltk` with Python.

```console
pip install nltk
python -c "import nltk; nltk.download('words')"
cp ~/nltk_data/corpora/words/en words.txt
```

### Quickstart

```console
gcc main.c -o main
./main -f words.txt
```
