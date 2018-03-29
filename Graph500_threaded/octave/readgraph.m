function [G] = readgraph( filename )
%
% [G] = readgraph( filename )
%
idebug = 1;

S = load(filename,'-ascii');
n = S(1,1);
nedge = S(1,2);
m = size(S,1);
ii  = S(2:m,1)+1;
jj = S(2:m,2)+1;
clear S;

G = sparse( ii,jj, ones(nedge,1), n,n);
G = G + G';
G = (G ~= 0);

nisolate = sum( sum( G, 2 ) == 0);
nisolate  = trace(nisolate); %  convert from (1,1) to scalar
if (idebug >= 1),
  disp(sprintf('graph has %d vertices, %d edges %d true edges %d isolated vertices', ...
          n,  nedge, nnz( triu( G,1) ), nisolate ) );
end;
