/*
   This file is part of SSCA1.

   Copyright (C) 2008-2015, UT-Battelle, LLC.

   This product includes software produced by UT-Battelle, LLC under Contract No.
   DE-AC05-00OR22725 with the Department of Energy.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the New BSD 3-clause software license (LICENSE).

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   LICENSE for more details.

   For more information please contact the SSCA1 developers at:
   bakermb@ornl.gov
*/

#include <pairwise_align.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <util.h>

int verify_alignment(good_match_t *A, int maxDisplay)
{
  int score;
  int retval = 0;
  
  if(A->bestLength == 0)
  {
    printf("\nFound no acceptable alignments.\n");
    return retval; // returning sucess here because the alignment didn't actually fail...
  }
  else
  {
    printf("\nFound %lu acceptable alignments with scores from %i to %i.\n",
           A->bestLength, (int)A->bestScores[0], (int)A->bestScores[A->bestLength-1]);
    if(maxDisplay > 0)
    {
      printf("\nStarting   Amino     Codon           Ending");
      printf("\nposition   acids     bases           position\n");
    }   
  }
  for(int m=0; m < A->bestLength; m++)
  {
    score = simple_score(A, A->bestSeqs[m].main, A->bestSeqs[m].match);
    if(score != A->bestScores[m])
    {
      retval = 1;
      printf("\nverifyAlignment %i failed; Kernel 1 reported %i versus Kernel 2 reported %i:\n---------------------------\n",
             m, (int)A->bestScores[m], (int)score);
    }
    else if(m < maxDisplay)
    {
      printf("\nverifyAlignment %i, succeeded; score %i:\n", m, (int)A->bestScores[m]);
    }
    if(m < maxDisplay)
    {
      char main_acid_chain[A->bestSeqs[m].main->length+1];
      char match_acid_chain[A->bestSeqs[m].match->length+1];
      char main_codon_chain[A->bestSeqs[m].main->length*3+1];
      char match_codon_chain[A->bestSeqs[m].match->length*3+1];

      memset(main_acid_chain, '\0', A->bestSeqs[m].main->length+1);
      memset(match_acid_chain, '\0', A->bestSeqs[m].match->length+1);
      memset(main_codon_chain, '\0', A->bestSeqs[m].main->length*3+1);
      memset(match_codon_chain, '\0', A->bestSeqs[m].match->length*3+1);

      assemble_acid_chain(A, main_acid_chain, A->bestSeqs[m].main, A->bestSeqs[m].main->length);
      assemble_acid_chain(A, match_acid_chain, A->bestSeqs[m].match, A->bestSeqs[m].match->length);

      assemble_codon_chain(A, main_codon_chain, A->bestSeqs[m].main, A->bestSeqs[m].main->length);
      assemble_codon_chain(A, match_codon_chain, A->bestSeqs[m].match, A->bestSeqs[m].match->length);

      printf("%7ld  %s  %s  %7ld\n%7ld  %s  %s  %7ld\n", 
             A->bestStarts[0][m], main_acid_chain, main_codon_chain, A->bestEnds[0][m],
             A->bestStarts[1][m], match_acid_chain, match_codon_chain, A->bestEnds[1][m]);
    }
  }
  return retval;
}

/*
 * Generate a typical matching pair.
 *
 * With a small change this recursive approach can be used to generate all
 * possible matches.  We only need to identify a single match.  C implementation
 * is limited in recursion only by memory.
 *
 * INPUTS
 *   A              [good_match_t] used to extract the hyphen
 *   sizeT          [int] size of the matrix T
 *   T              [int[][]] matrix used to navigate with
 *   ei,ej          [int] base position for codons in the sequence
 *   i,j            [int] subsequence end coordinates, used recursively
 *   dir            [int] 1 when extending a gap in main (down),
 *                        2 when extending a gap in match (right), else 0
 *   depth          [int] used to decide how big the outputs will be and for debugging
 * OUTPUT
 *   rs             [i, j] coordinates of end of path or -1 if there is no path
 *   ri             [int[M]] main sequence
 *   rj             [uint8 row vector] match sequence
 */

  /* Matlab allows for a = [1 2] and b = [0 a] and b will be [0 1 2].  Obviously we can't have that in C. */
  void tracepath(good_match_t *A, int sizeT, int *T, int ei, int ej, int i, int j, int dir, int rs[2], seq_data_t *sequence, int depth) {
    codon_t Ci, Cj;

  /* debug code */
  /*
    if(depth > sizeT)
    {
    printf("Recursion went to an insane depth, aborting.");
    abort();
    }
  */

  if(i == -1 || j == -1) /* Done */
  {
    rs[0] = i + 1;
    rs[1] = j + 1;
    sequence->main = alloc_local_seq(depth);
    sequence->match = alloc_local_seq(depth);
    return;
  }

  if(dir == 0 || (dir & T[index2d(i,j,sizeT)])) // if we aren't skipping and it isn't part of a gap
  {                               // then we fan out from here
    dir = T[index2d(i,j,sizeT)] >> 2;
  }
  
  if(dir == 0) // if we didn't find a sequence
  {
    rs[0] = -1; // Using -1 to mean 'does not exist'
    rs[1] = -1;
  }

  fetch_from_seq(A->seqData->main,ei-i,&Ci);
  fetch_from_seq(A->seqData->match,ej-j,&Cj);

  if(dir & 4)
  {
    tracepath(A, sizeT, T, ei, ej, i-1, j-1, 0, rs, sequence, depth+1);
    if(rs[0]!=-1)
    {
      sequence->main->sequence[depth] = Ci;
      sequence->match->sequence[depth] = Cj;
      return;
    }
  }

  if(dir & 2)
  {
    tracepath(A, sizeT, T,  ei, ej,i-1, j, 2, rs, sequence, depth+1);
    if(rs[0]!=-1)
    {
      sequence->main->sequence[depth] = Ci;
      sequence->match->sequence[depth] = A->simMatrix->hyphen;
      return;
    }
  }

  if(dir & 1)
  {
    tracepath(A, sizeT, T, ei, ej, i, j-1, 1, rs, sequence, depth+1);
    if(rs[0]!=-1)
    {
      sequence->main->sequence[depth] = A->simMatrix->hyphen;
      sequence->match->sequence[depth] = Cj;
      return;
    }
  }
}



/*
 * Scan from the current end point -- return if expected match is found.
 *
 * This variant of the Smith-Waterman algorithm starts at an end-point pair and
 * proceeds diagonally up and to the left, searching for the matching
 * start-point pair.  The outer loop goes right to left horizontally across the
 * matchSeq dimension.  The inner loop goes diagonally up and right, working
 * backward through the main sequence and simultaneously forward through the
 * match sequence.  The loops terminate at the edges of the square of possible
 * matches; the algorithm terminates at the first match with the specified
 * score.
 *
 * This variant differs from the Kernel 1 variant in that its initial point
 * is fixed (the end-point pair), and that its goal is known.  To find this
 * goal it may be necessary for intermediate values of the score to go
 * negative, which is impossible for Kernel 1.  However such matches are
 * rejected during traceback and may cause the end-point pair to be rejected.
 * For example the match
 *       ABCDEF------G
 *       ABCDEFXXXXXXG
 * might be found by Kernel 1 since its score stays positive scanning right to
 * left and its end-point pair is distinct from the F/F end-point pair.  But
 * scaning left to right the G/G match is not enough to prevent the gap from
 * taking the score negative.  (In this case, this match would also be rejected
 * since its start-point pair A/A matches the better ABCDEF/ABCDEF match.)
 */

void doScan(good_match_t *A, int *bestR, int minSeparation, int report) {
  score_t goal = A->goodScores[report];
  index_t ei = A->goodEnds[0][report];
  index_t ej = A->goodEnds[1][report];

  //codon_t *mainSeq = A->seqData->main->sequence;
  //codon_t *matchSeq = A->seqData->match->sequence;
  seq_t *main_seq = A->seqData->main;
  seq_t *match_seq = A->seqData->match;
  int gapExtend = A->simMatrix->gapExtend;
  int gapFirst = A->simMatrix->gapStart + A->simMatrix->gapExtend;        // total penalty for first codon in gap

  int m = ei > ej ? ei : ej;
  int *V[2];
  int *E;
  int *F;
  int s, fj, fi, lj, v, dj, di, e, f, G;
  int compare_a, compare_b;
  int rs[2];
  codon_t main_codon, match_codon;

  int sizeT = 2 * (A->simMatrix->matchLimit > A->seqData->max_validation ? A->simMatrix->matchLimit : A->seqData->max_validation);
  int *T = malloc(sizeof(int) * sizeT * sizeT);
  memset(T, '\0', sizeof(int) * sizeT * sizeT);

  V[0] = malloc(sizeof(int) * (m+1));
  V[1] = malloc(sizeof(int) * (m+1));
  E = malloc(sizeof(int) * m);
  F = malloc(sizeof(int) * m);

  //initialize the V, E, and F arrays.  We use the smallest possible value that won't be underflowed by the algorithm.
  for(int idx=0; idx < m; idx++)
  {
    V[0][idx] = INT_MIN + gapFirst;
    V[1][idx] = INT_MIN + gapFirst;
    E[idx] = INT_MIN + gapFirst;
    F[idx] = INT_MIN + gapFirst;
  }

  V[0][m] = INT_MIN + gapFirst;
  V[1][m] = INT_MIN + gapFirst;

  fetch_from_seq(main_seq,ei,&main_codon);
  fetch_from_seq(match_seq,ej,&match_codon);

  s = A->simMatrix->similarity[main_codon][match_codon];

  //special case for the first point
  if(s==goal) return;

  V[0][1] = s;
  E[0] = s - gapFirst;
  F[0] = s - gapFirst;
  T[0] = 19;
  fj = ej - 1; // final point on the diagnal
  fi = ei; 
  lj = ej; // first point on the diagnal
  v = 1;
  seq_data_t test_seq;

  while(fi > 0) // loop over diagnal starting positions
  {
    dj=fj;
    di=fi;
    e=ei-di;
    f=ej-dj;

    while(dj <= lj && di > 0 && e < sizeT && f < sizeT)
    {
      fetch_from_seq(main_seq,di,&main_codon);
      fetch_from_seq(match_seq,dj,&match_codon);
      G = A->simMatrix->similarity[main_codon][match_codon] + V[v][f];
      // find the very best weight ending with this pair 
      compare_a = E[e] > F[f] ? E[e] : F[f];
      s = G > compare_a ? G : compare_a;
      V[v][f+1] = s;

      if(s>0) // if score is okay, track this path
      {
      	T[index2d(e,f,sizeT)] = 4*(s == E[e]) + 8*(s == F[f]) + 16*(s == G);
      }
      else // eliminate this path
      {
	      T[index2d(e,f,sizeT)] = 0;
      }

      if(s == goal)
      {
        //discard if start is too close to a better sequence
	      for(int r = 0; r < *bestR; r++)
	      {
          compare_a = abs(di - A->bestStarts[0][r]);
          compare_b = abs(dj - A->bestStarts[1][r]);
          if((compare_a > compare_b ? compare_a : compare_b) < minSeparation)
          {
            //printf("Start too close to %i: report %i discarded\n", r, report);
            goto out;
          }
        }

        tracepath(A, sizeT, T, ei, ej, e, f, 0, rs, &test_seq, 0);

        if(rs[0] == -1 && rs[1] == -1) // tracepath returns -1 in rs if there is no path
        {
          //printf("No path: report %d discarded\n", report);
          goto out;
        }

        // record the result and return
        A->bestStarts[0][*bestR] = di;
        A->bestStarts[1][*bestR] = dj;
        A->bestEnds[0][*bestR] = ei;
        A->bestEnds[1][*bestR] = ej;
        A->bestScores[*bestR] = goal;
        memcpy(&(A->bestSeqs[*bestR]), &test_seq, sizeof(seq_t));
        (*bestR) = (*bestR)+1;
        goto out; // sucess
      }
      s = s - gapFirst;

      // find the best weight assuming a gap in matchSeq
      E[e] = ((E[e]-gapExtend > s) ? E[e]-gapExtend : s );

      // find the weight assuming a gap in mainSeq
      F[f] = ((F[f]-gapExtend > s) ? F[f]-gapExtend : s );
      T[index2d(e,f,sizeT)] = T[index2d(e,f,sizeT)] + (E[e] == s) + 2*(F[f] == s);

      dj++;
      di--;
      e++;
      f--;
    }
    v = 1 - v;
    if(fj != 0)
    {
      fj--;
    }
    else
    {
      fi--;
    }
  }
  // see the note in the function header regarding sequences that could not be found
  printf("Could not find sequence %i.\n", report);
out:
  free(F);
  free(E);
  free(V[0]);
  free(V[1]);
  free(T);
}

/*
 * This function uses a variant of the Smith-Waterman dynamic programming
 * algorithm to locate each actual aligned sequence from its end points and
 * score as reported by Kernel 1.  Some of end-points pairs may be rejected,
 * primarily because their matching start-points fall within a specified
 * interval of a better match.  Only the best maxReports matches are reported.
 *
 * While the start-points are being located, a record is kept of the
 * alternatives at each point.  Then the recursive tracepath function is
 * used to locate the match to report; there may be one or more equally
 * valid matches and if so the first one found is reported.
 *
 * For a detailed description of the SSCA #1 Optimal Pattern Matching problem,
 * please see the SSCA #1 Written Specification.
 *
 * INPUT
 * A                - [good_match_t] results from pairwiseAlign
 *   seqData        - [seq_data_t] data sequences created by genScalData()
 *     main         - [char pointer] first codon sequence
 *     match        - [char pointer] second codon sequence
 *     maxValidation- [int] longest matching validation string.
 *   simMatrix      - [sim_matrix_t] codon similarity created by genSimMatrix()
 *     similarity   - [int [64][64]] 1-based codon/codon similarity table
 *     aminoAcid    - [char[65]] 1-based codon to aminoAcid table
 *     bases        - [char [4]] 1-based encoding to base letter table
 *     codon        - [char [64][3]] 1-based codon to base letters table
 *     encode       - [int [128]] aminoAcid character to last codon number
 *     hyphen       - [char] encoding representing a hyphen (gap or space)
 *     exact        - [int] value for exactly matching codons
 *     similar      - [int] value for similar codons (same amino acid)
 *     dissimilar   - [int] value for all other codons
 *     gapStart     - [int] penalty to start gap (>=0)
 *     gapExtend    - [int] penalty to for each codon in the gap (>0)
 *     matchLimit   - [int] longest match including hyphens
 *   goodEnds       - [int[M][2]] M matches; main/match endpoints
 *   goodScores     - [int[M]] the scores for the goodEnds
 * maxReports       - [int] maximum number of endpoints reported
 * minSeparation    - [int] minimum startpoint separation in codons
 *
 * OUTPUT
 * A
 *   bestStarts       - [int[M][2]] main/match startpoints
 *   bestEnds         - [int[M][2]] main/match endpoints
 *   bestSeqs         - [seq_t[M]] main/match sequences
 *     main           - [int[M]] codon sequence from main sequence
 *     match          - [int[M]] codon sequence from match sequence
 *     length         - length of the codons, including gaps
 *   bestScores       - [int[M]] the scores for the bestSeqs
 *   bestLength       - [int] value of M.
 */

/* 
 * This function really just sets up memory and invokes the main loop until we have the required number of reports.
 * The real meat is in doScan.
 */
void scanBackward(good_match_t *A, int maxReports, int minSeparation)
{
  // Preallocate working storage, thinking of cleaning up large chunks of this.

  A->bestStarts[0] = (index_t *)malloc(maxReports*sizeof(index_t)*2);
  A->bestStarts[1] = (index_t *)malloc(maxReports*sizeof(index_t)*2);
  A->bestEnds[0] = (index_t *)malloc(maxReports*sizeof(index_t)*2);
  A->bestEnds[1] = (index_t *)malloc(maxReports*sizeof(index_t)*2);
  A->bestScores = (score_t *)malloc(maxReports*sizeof(score_t)*2);
  A->bestSeqs = (seq_data_t *)malloc(maxReports * sizeof(seq_data_t)*2);

  memset(A->bestStarts[0], '\0', sizeof(index_t) * maxReports);
  memset(A->bestStarts[1], '\0', sizeof(index_t) * maxReports);
  memset(A->bestEnds[0], '\0', sizeof(index_t) * maxReports);
  memset(A->bestEnds[1], '\0', sizeof(index_t) * maxReports);
  memset(A->bestScores, '\0', sizeof(score_t) * maxReports);
  memset(A->bestSeqs, '\0', sizeof(seq_data_t) * maxReports);

  int bestR = 0;

  for(int report=0; report < A->numReports; report++)
  {
    doScan(A, &bestR, minSeparation, report);
    if(bestR==maxReports)
      break;
  }
  A->bestLength=bestR;
}

