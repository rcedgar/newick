"Make a subset tree given file with leaf labels, one per line (labels\n"
"do not need to be a subtree, the tree is collapsed as needed):\n"
"    newick -input tree.newick -labels labels.txt -output subset.newick\n"
"\n"
"Get leaf labels:\n"
"    newick -getlabels tree.newick -output labels.txt\n"
"\n"
"Report miscellaneous information about a Newick file:\n"
"    newick -stats trees.newick\n"
"\n"
"Calculate Robinson-Foulds (R-F) distance between two trees:\n"
"    newick -rofo tree1.newick -tree2 tree2.newick -log rofo.log\n"
"\n"
"Calculate all-vs-all R-F distances between trees in Newick file:\n"
"    newick -rofos trees.newick -log rofos.log\n"
"\n"
"Re-label trees, labels.tsv tab-separated with #1=old_label #2=new_label:\n"
"    newick -relabel trees.newick -labels2 labels.tsv -output relabeled_trees.newick\n"
"\n"
"Add integer node number labels to internal nodes:\n"
"    newick -intlabel tree.newick -output intlabel.newick\n"
"\n"
"Root by outgroup, specify labels.txt with leaf labels of outgroup or GroupName which\n"
"is a substring of the outgroup labels, e.g. phylum name if format is A1234.Phylum:\n"
"    newick trees.newick [-labels labels.txt | -outgroup GroupName] -output rooted.newick\n"
"\n"
"Convert tab-separated to Newick:\n"
"    newick -tsv2newick tree.tsv -output tree.newick\n"
"\n"
"Convert Newick to tab-separated:\n"
"    newick -newick2tsv tree.newick -output tree.tsv\n"
"\n"
"Ladderize trees by rotating internal nodes so that larger subtree is always the \n"
"left (default) or right subtree:\n"
"    newick -ladderize trees.newick -output ladderized.newick [-right]\n"
"\n"
"Split tree into N roughly equal-sized subtrees (clusters), output is N files\n"
"named prefixi, i=1..N containing labels for each subtree:\n"
"    newick -split tree.newick -n N -prefix prefix\n"
"\n"
"Convert trees to cladograms (leaves equidistant from root):\n"
"    newick -clado trees.newick -output clado.newick\n"
"\n"
"Calculate edge confidence values from set of bootstrapped trees:\n"
"    newick -conf tree.newick -trees replicates.newick -output conftree.newick\n"
"\n"
"Condense a tree by identifying best-fit nodes for each feature group and making\n"
"a tree of just those nodes; unary edges are collapsed by summing lengths and \n"
"taking max confidence, leaves are labeled with features (e.g. phylum names):\n"
"    newick -condense trees.newick -features features.tsv -output condensed.newick\n"
"\n"
"Extract just the branching order by collapsing unary nodes, deleting all edge lengths\n"
"and deleting all confidence values (all internal node labels removed):\n"
"    newick -topo trees.newick -output topos.newick\n"
"\n"
"Delete one or more leaves and collapse any resulting unary nodes, useful e.g. for\n"
"deleting outgroup to simplify figure:\n"
"    newick -deleteleaves trees.newick [-label OutgroupName | -labels labels.txt] -output .newick\n"
"\n"
"Draw one tree or several trees with optional coloring of edges:\n"
"    newick -draw tree.newick -svg figure.svg\n"
"    newick -drawf tree.newick -features features.tsv -colors colors.tsv -svg figure.svg\n"
"    newick -drawfs trees.newick -features features.tsv -colors colors.tsv -svg figure.svg\n"
"\n"
"    -features is tsv file with #1 leaf_label #2 feature_name (e.g. phylum).\n"
"    -colors is tsv file with #1 feature_name #2 color, where color is any valid svg color,\n"
"      can be rgb, hex or name e.g. red.\n"
"\n"
"    -default_color color\n"
"        Color for unlabeled edges (default gray).\n"
"    -title text\n"
"        Title text.\n"
"    -title_font_size n\n"
"        Title font size (default 10).\n"
"    -unitlengths \n"
"        Treat all edge lengths as 1 (phylogram).\n"
"    -strokewidth n\n"
"        Line width for edges (default 1).\n"
"    -tree_width n\n"
"        Width of tree (default 1000).\n"
"    -tree_height n\n"
"        Height of tree (default 1000).\n"
"    -tree_spacing n\n"
"        Space between trees (default 300).\n"
"    -trees_per_row n\n"
"        Number of trees per row in figure (default 4).\n"
"    -legend legend.svg\n"
"        Legend showing features (e.g. phylum names) and colors.\n"
