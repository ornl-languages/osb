function [vi]  = map_vertex( pvi, state )
%
%  vi  = map_vertex( pvi, state )
%
%  ------------------------------------
%  given a probability value 0 <= pvi <= 1,
%  find a vertex base on the degree list
%  ------------------------------------
%
idebug = 0;
use_rand = state.use_rand;

is_column_vector = (size(pvi,2) == 1);

prob_wdl = state.prob_wdl;
csum_dl = state.csum_dl;

if (is_column_vector),
   prob_wdl = reshape( prob_wdl,  length(prob_wdl),1);
   csum_dl = reshape( csum_dl, length(csum_dl),1);
else
   prob_wdl = reshape( prob_wdl,  1,length(prob_wdl));
   csum_dl = reshape( csum_dl, 1,length(csum_dl));
end;
  
nbins = length(prob_wdl) - 1;

% ----------------------------------------------------
% find ibin such that
% (prob_wdl(ibin) <= pvi ) & (pvi <= prob_wdl(ibin+1))
% ----------------------------------------------------
use_binary_search = 1;
if (use_binary_search),
  ibin_lo = ones(size(pvi)); 
  ibin_hi = nbins*ones(size(pvi));
  nsteps = ceil(  log(nbins)/log(2)) ;

  for istep=1:nsteps,
    ibin = floor( (ibin_hi + ibin_lo)/2 );
%     found = (prob_wdl(ibin) <= pvi) & (pvi <= prob_wdl(ibin+1));
%     if (all(found)),
%         break;
%     end;
    if (idebug >= 1),
      disp(sprintf('before: ibin_lo %g (%g) ibin %g (%g)  ibin_hi %g (%g) ', ...
           ibin_lo,prob_wdl(ibin_lo),  ibin,pvi,  ibin_hi, prob_wdl(ibin_hi) ));
    end;

    is_lower = (pvi < prob_wdl(ibin) );
    ibin_hi = merge( is_lower,ibin-1,ibin_hi );
    is_upper = (pvi > prob_wdl(ibin+1));
    ibin_lo = merge( is_upper,ibin+1,ibin_lo );

    if (idebug >= 1),
      disp(sprintf('after: is_lower %g is_upper %g ibin_lo %g ibin_hi %g ', ...
                 is_lower, is_upper, ibin_lo, ibin_hi ));
    end;

  end;
              
else
  ibin = (find( (prob_wdl(1:nbins) <= pvi) & ...
                 (pvi <= prob_wdl(2:(nbins+1))) )  );
end;
isok  = (1 <= ibin) & (ibin <= nbins) & ...
        (prob_wdl(ibin) <= pvi) & (pvi <= prob_wdl(ibin+1)); 
if (~all(isok)),
  disp(sprintf(...
    'map_vertex: pvi %g ibin %g prob_wdl(ibin) %g prob_wdl(ibin+1) %g', ...
                 pvi,   ibin,   prob_wdl(ibin),  prob_wdl(ibin+1) )); 
  if (idebug >= 2),
    disp('prob_wdl');
    prob_wdl
  end;
  error(sprintf('map_vertex: pvi %g ', pvi  ));
end;

% -------------------------------------------------
% the bin "ibin" correspond to vertices  [vs .. ve]
% -------------------------------------------------
vs = csum_dl(ibin)+1;
ve = csum_dl(ibin+1);
vsize = (ve - vs + 1);

% -----------------------------------------
% assume equal chance of selecting [vs..ve]
% -----------------------------------------
if (use_rand),
  ppvi = rand(size(pvi));
else

  plo = prob_wdl(ibin);
  phi = prob_wdl(ibin+1);
  ppvi = (pvi-plo)./(phi-plo);
end;

  imin = vs;
  imax = ve;
  vi = imin + floor( (imax-imin+1 ) .* ppvi );

