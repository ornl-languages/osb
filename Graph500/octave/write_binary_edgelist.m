function [dummy] = write_binary_edgelist( filename, edgelist )
%
% [dummy] = write_binary_edgelist( filename, edgelist )
%
fid = fopen(filename,'w');
%
% header is gnvertex  gnedges
%
dummy = 0;

n1 = size(edgelist,1);
n2 = size(edgelist,2);



isok = (n1 == 2); 
if (~isok),
  error(sprintf('write_binary_edgelist: wrong shape for edgelist %d %d', ...
                                                                 n1, n2));
  return;
end;
nedge = size(edgelist,2);
min_vertex = min( edgelist(:));
nvertex = max( edgelist(:) );
if (min_vertex == 0),
  nvertex = nvertex + 1;
end;

   
[fid,msg] = fopen( filename, 'wb');
isok = (fid >= 0);
if (~isok),
  error( sprintf('write_binary_edgelist: problem opening file %s: %s', ...
                   filename, msg ));
  return;
end;


ibuf = [nvertex, nedge ];
precision = 'int64';
icount = fwrite(fid, ibuf, precision );
isok = (icount == 2);
if (~isok),
  error( sprintf('write_binary_edgelist: problem writing header in file %s', ...
                 filename ));
  return;
end;

icount = fwrite(fid, edgelist, precision);
isok = (icount == (2 * nedge) );
if (~isok),
  error(sprintf('write_binary_edgelist: problem writing data in file %s', ...
            filename));
  return;
end;

istat = fclose(fid);
isok = (istat == 0);
if (~isok),
  error(sprintf('write_binary_edgelist: problem closing file %s', ...
            filename));
  return;
end;





