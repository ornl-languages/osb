function void = write_graph( G, filename, use_ij_in )
% void = write_graph( G, filename )
% 
% write out graph
%

use_ij = 1;
if (nargin >= 3),
  use_ij = use_ij_in;
end;

% ---------------------------------------------
% write out only lower triangular part as edges
% ---------------------------------------------
[ii,jj,aij] = find(  tril(G,-1) );
nvertex = size(G,1);
nedges = length(ii);

% ------------------
% write in text mode
% ------------------
mode = 'wt';
[fid,msg] = fopen( filename, mode );
if (length(msg) >= 1),
   error(sprintf('write_graph: error in opening file  %g msg %g', ...
               filename,   msg ));
   return;
end;
frewind(fid);

% ------------
% write header
% ------------
fdisp(fid,'% nvertex,  nedges , edges in lower triangular matrix');
fdisp(fid,sprintf('%d %d', nvertex, nedges ));

if (use_ij),
  ij = [ reshape(ii,1,nedges); ...
         reshape(jj,1,nedges) ];
  fprintf(fid,'%d %d\n', ij );

else
  for iedge=1:nedges,
    fdisp(fid,sprintf('%d %d ', ii(iedge), jj(iedge)));
  end;
end;


fclose(fid);
