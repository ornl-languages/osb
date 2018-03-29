filename = "file.s14"
[n,nedge,ij] = read_edgelist(filename);

dummy = write_binary_edgelist( strcat(filename,'.bin'), ij );

