function [deg_dist] = ij2deg_dist(ij)
%
% [deg_dist] = ij2deg_dist(nvetex, nedge, ij)
%
% generate the degree distribution from edge list 
%
%  note ij is size (nedge+1) by 2, the first line
%  contains nvetex = ij(1,1),   nedge = ij(1,2)
%

% ---------
% 1st line is number of vertices, number of edges
% ---------
nvertex = ij(1,1); nedge = ij(1,2);
n = nvertex;

m = nedge + 1;
A = sparse( ij(2:m,1)+1, ij(2:m,2)+1, ones(nedge,1), nvertex,nvertex);

% ---------------------
% make matrix symmetric
% ---------------------
A = (A + A')/2;

% -------------------
% make entries 0 or 1
% -------------------
A = (A ~= 0);

% ----------------------------------------------
% remove self-loops so that diagonal is all 0's
% ----------------------------------------------
A = A - spdiag( diag(A,0),0);

% ------------------------------------------------------
% generate degree distribution by constructing histogram
% ------------------------------------------------------
deg_list = sum( A );
nbins = max( deg_list ) + 1;
deg_distribution = hist( deg_list, nbins );

deg_dist = zeros(n,2);
deg_dist(1:n,1) = 0:(n-1);
deg_dist(1:nbins, 2) = deg_distribution(1:nbins);
