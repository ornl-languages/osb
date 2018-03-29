
SSCA1 OVERVIEW
--------------

The sequence alignment is the central functionality of genetic analysis and
many other Bioinformatics applications.  It infers the structural and
functional relationship between the DNA/RNA or protein sequences, and finds
similarity between the query and reference sequences.  The sequence matching
functionality, essentially a pattern matching function, has wider
applications besides Bioinformatics.  One of the most popular algorithms for
sequence matching is the Smith-Waterman algorithm, which employs a dynamic
programming approach to find local alignment, which finds local regions with
high levels of similarity.  Given the importance of this algorithm and its
wider application, it is included in many benchmark suites including
Scalable Synthetic Compact Applications 1 (SSCA1) and BioParallel, and is
used in tools for analyzing next generation sequencing data such as
SNPTools.

Like many scientific simulations and workloads, the Smith-Waterman algorithm
execution is constrained by the availability of resources.  The algorithm
complexity is bounded by O(mn), where m is the length of the main sequence
and n is the lengths of the match sequence.  Given the importance of the
algorithm and the asymptotic runtime of the algorithm, there are various
parallelization and optimization strategies to address the resource problem.
Researchers have parallelized the Smith-Waterman algorithm by implementing
the algorithm using parallel programming models such as MPI and BSP models
and use distributed memory systems for execution.  For multicore systems, it
has been optimized using hybrid programming paradigms such as MPI+OpenMP.
With GPUs providing more performance per watt, the GPU based distributed
memory systems have become ubiquitous.  To take advantage of this computing
architecture, researchers have developed a parallelized implementation of
the Smith-Waterman algorithm for many thread systems such as GPUs.

The Smith-Waterman algorithm looks for sequence subsets that best match in
two large sequences.  We ﬁx the length of the main and match sequences to be
the same, so the algorithm effectively runs in O(n^2) time.

The first step is to generate a similarity matrix.  This similarity matrix
decides if a pair of codons, a triplet of adjacent DNA nucleotides that code
for proteins, are either exact matches, similar matches that are distinct
but serve the same function, or dissimilar matches.  This can be generalized
as a function sim(a, b) that will return an exact match score, a similar
score, or a no match score.

This algorithm works with a dynamic programming matrix A.  This matrix is
formed for each element A_(ij) by comparing element i in the main sequence
and j in the match sequence using the sim(a, b) function to derive a score
that is added to element A_((i-_1)(j-1)) of the score matrix representing
the score if the current sequence were matched.  A score is derived for a
gap in the main sequence by subtracting a gap penalty from element
A_(i(j_1)) in the score matrix.  A score for a gap in the match sequence is
also derived by taking element A_((i - 1)j) and subtracting the gap penalty.
These scores, for the gap in main, the gap in match, and a similarity value
between main and a match, are all compared and the highest score is the
score used for element A_(ij) in the matrix to a minimum score of 0.

