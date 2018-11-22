# Cracking Pivot Selection

## Introduction
	In this project, I explore different strategies for cracking pivot selection and test them through a variety of column distributions and query workloads. In 'Standard' Cracking [Citation] the cracking pivots are the same as the query predicates. Stochastic Cracking [Citation] uses random values from within the piece where the query predicate falls. 
	
	Four different pivot types are implemented:
	* Exact Query Predicate: Similar to Standard Cracking, uses the exact query predicates as the cracking pivots;
	* Within Query Predicate Piece: Similar to Stochastic Cracking, cracks the piece where the query predicate falls in;
	* Within Query: Crack the set of pieces that compose a query;
	* Withing Column: Crack the column.

	Three different strategies for pivot selection are implemented (Note that they are not used for the Exact Query Predicate strategy):
	* Random: A random value is picked;
	* Median: An early-exit quick-sort iteration is used to retrieve the median of that piece;
	* Approximate Median: The value in the middle of the piece is used.

	In addition, the pieces that are going to be cracked are selected either on a RANDOM or BIGGEST PIECE strategy (Note that this is only used for "Within Query" and "Within Column" pivot types)


## Compiling/Testing/Running