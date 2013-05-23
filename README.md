Huffman Compression in C
========================

This is an implementation of a huffman code compression algorithm in C.

A background on what Huffman coding envolves is [available on Wikipedia](http://en.wikipedia.org/wiki/Huffman_coding).

Guide to use
------------

You can build the huffman encoder and decoder binaries with

```
make
```

To compress a file simply use:

```
./huffman file_to_compress compressed_file
```

And to decompress you can use:

```
./unhuffman compressed_file uncompressed_file
```

or

```
./huffman -u compressed_file uncompressed_file
``` 

Further options
---------------

It is possible to get some compression statistics using the ```-s``` option

```
./huffman -s file_to_compress compressed_file
```

It is also possible to take input from ```stdin```, which is defined as the filename ```-```

```
./huffman - compressed_file <file_to_compress
```

or to output to ```stdout```

```
./huffman file_to_compress | ./unhuffman -c - > uncompressed_file
```

Here we see that if we leave off the output file with ```huffman``` the output is assumed to be ```stdout```. To be explicit that you want to output to ```stdout``` you can use the option ```-c```.

