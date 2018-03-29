
% ----------------------------
% simple script to 
% compare erdos_renyi_generator
% by comparing with kronecker graph generator
% ----------------------------

scale = 14;
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

deg_of_vert_kron = sum( G - spdiag( diag(G,0),0) );



% -----------------------------
% use erdos_renyi graph generator
% -----------------------------
t1 = cputime;
ij = erdos_renyi_generator( scale, avg_deg );
t2 = cputime;
time_erdos_renyi_generator = (t2-t1);
nvertex = max(max(ij))+1;
nedges = size(ij,2);
disp(sprintf('erdos_renyi_generator took %g sec: nvertex %d nedges %d ', ...
		     time_erdos_renyi_generator,  nvertex,   nedges ));


G = sparse( ij(1,:) + 1, ij(2,:)+1, ones(1,size(ij,2)), nvertex,nvertex );
G = (G + G')/2;
G = (G ~= 0);

deg_of_vert_er = sum( G - spdiag( diag(G,0),0) );


clf;
subplot(2,1,1);
plot( 1:length(deg_of_vert_er), sort(deg_of_vert_er), 'r.-', ...
      1:length(deg_of_vert_kron), sort(deg_of_vert_kron),'b-' );
legend('ER','KRON');
title(sprintf('degree profile of vertices, scale=%d',scale));

subplot(2,1,2);
max_deg = max( deg_of_vert_er );
hist( deg_of_vert_er, max_deg );
title(sprintf('distribution of degrees for ER, %d verties', length(deg_of_vert_er)));

print('test1_erdos_renyi_generator.png','-dpng');
