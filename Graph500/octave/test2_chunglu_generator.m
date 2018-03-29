% ----------------------------
% simple script to 
% test chunglu graph generator
% by comparing with kronecker graph generator
% ----------------------------

scale = 18;
avg_deg = 16;
state.use_rand = 0;

% -----------------------------
% use kronecker graph generator
% -----------------------------
t1 = cputime;
ij = kronecker_generator( scale, avg_deg );
t2 = cputime;
time_kronecker_generator = (t2-t1);
nvertex = max(max(ij))+1;
nedges = size(ij,2);
disp(sprintf('kronecker_generator took %g sec: nvertex %d nedges %d ', ...
		     time_kronecker_generator,  nvertex,   nedges ));


G = sparse( ij(1,:) + 1, ij(2,:)+1, ones(1,size(ij,2)), nvertex,nvertex );
G = (G + G')/2;
G = (G ~= 0);

deg_of_vert = sum( G - spdiag( diag(G,0),0) );

filename = sprintf('kron_scale=%d.dat',scale);
write_graph(G,filename);
clear G;

max_degree = max(max( deg_of_vert ));

% -----------------------------------
% note there may be isolated vertices of degree 0
% -----------------------------------
idx_connected = find( deg_of_vert >  0);
nisolated = sum( deg_of_vert == 0);
disp(sprintf('there are %g isolated vertices with degree zero', ...
              nisolated ));

% ------------------------
% remove isolated vertices
% and generate degree sequence list
% ------------------------

deg_list = hist( deg_of_vert(idx_connected), max_degree );




nv = nvertex - nisolated;
t1 = cputime;
ij2 = chunglu_generator( deg_list , state);
t2 = cputime;
time_chunglu_generator = (t2-t1);

nvertex = max(max(ij2))+1;
nedges = size(ij2,2);
disp(sprintf('chunglu took %g sec: nvertex %d nedges %d ', ...
		time_chunglu_generator,   nvertex,   nedges ));

% ------------------
% generate new graph
% ------------------



nedges = size(ij2,2);
nvert2 = max( max(ij2) ) + 1;

G2 = sparse( ij2(1,:)+1, ij2(2,:)+1, ones(1, nedges),nvert2,nvert2);
G2 = (G2+G2')/2;
G2 = (G2 ~= 0);

deg_of_vert2 = sum( G2 - spdiag( diag(G2,0),0) );

%
% ------------
% write out G2
% ------------
filename = sprintf('cl_scale=%d.dat',scale);
write_graph( G2, filename );
