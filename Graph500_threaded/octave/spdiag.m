function D = spdiag( v, idiag)
%
% D = spdiag( v, idiag)
%
% generate a sparse matrix with a single off-set diagonal
% 
m = length(v);
n = m + abs(idiag);
if (idiag < 0),
  D = sparse( (1:m) + abs(idiag), (1:m), v(1:m), n,n);
else
  D = sparse( (1:m), (1:m) + idiag, v(1:m), n, n);
end;
