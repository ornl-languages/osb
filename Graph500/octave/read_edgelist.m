function [n,nedge, ij]  = read_edgelist(filename)
%
%  [n,nedge, ij]  = read_edgelist(filename)
%
fid = fopen(filename,'r');

header = fscanf(fid, '%g', [2,1]);
n = header(1);
nedge = header(2);
ij = fscanf(fid,'%g', [2,nedge]);
fclose(fid);


