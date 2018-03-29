function [edgelist] = chunglu_generator( degree_list, state)
%
% [edgelist] = chunglu( degree_list )
%
% generate graph using chunglu method
%
% degree_list(1:maxdegree)  shows number of vertices for 
% each degree value
%
% degree_list(1) is number of vertices with degree 1
% degree_list(2) is number of vertices with degree 2
% ...
% --------------------------------------
% Note: a node with degree zero is an isolated node
% consider generating the isolated nodes separately
% here consider only non-isolated nodes
% --------------------------------------

% 
% e.g. degree_list = [ 2, 3, 1 ]
% 2 vertices of degree 1
% 3 vertices of degree 2
% 1 vertices of degree 3
%
% wdl = [ 1*2, 2*3, 3*1 ] 
% wdl = [   2,   6,   3 ] 
% sum_wdl = 2 + 6 + 3 
%         = 11
% 
% csm_dl = [0,  dl(1), dl(1) + dl(2), dl(1)+dl(2)+dl(3) ]
%        = [0,      2,   2   +  3   ,   2  + 3   + 1    ] 
%        = [0,      2,             5,                  6]
%
% prob_wdl = [0,  [2,  2+6,  2+6+3]/11 ]
% prob_wdl = [0,   2/11,  8/11,  11/11 ]
% 
max_degree = length( degree_list );
total_vertices = sum( degree_list );
wdl = degree_list .* (1:max_degree);
total_edges = sum( wdl )/2;

% --------------------
% in one implementation of Chung-Lu
% if a vertex has degree 10, then 10 copies of the vertex is 
% replicated and put into a list
% but this can be very expensive in memory
% an alternative is an approximate sampling base on the weighted
% probability
% --------------------
csum_dl = [0,cumsum( degree_list )];

csum_wdl = cumsum( wdl );
sum_wdl = csum_wdl( length(csum_wdl) );
prob_wdl = [0, cumsum(wdl) ./sum_wdl  ];

% ---------------------------------------------
% use data structure 'state' to pass parameters
% ---------------------------------------------
state.prob_wdl = prob_wdl;
state.csum_dl = csum_dl;

nb = 1024;
for istart=1:nb:total_edges,
   iend = min(total_edges, istart+nb-1);
   isize = iend - istart + 1;

   % ------------------------------
   % find vertices vi, vj to form edge (vi,vj)
   % ------------------------------
   i=1:isize;
     iedge = (istart-1) + i;

     pvi = rand(isize,1);
     pvj = rand(isize,1);

     vi = map_vertex(pvi, state );
     vj = map_vertex(pvj, state );

     edgelist(1,iedge) = min(vi,vj);
     edgelist(2,iedge) = max(vi,vj);
end;

% -----------------------------------------
% assume vertex number start from 0
% to be compatible with kronecker_generator
% -----------------------------------------
edgelist = edgelist  - 1;




