% ------------------------------
% simple matlab function to form sparse matrix from list of edges
% note edge number in file is 0-based
% ------------------------------
function G = read_graph( filename )
% G = read_graph( filename )
%
G2 = load( filename );
G2 = G2 + 1;
n = max( max(G2) );
G = sparse( G2(:,1), G2(:,2), ones( size(G2,1),1), n,n );


