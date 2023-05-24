#ifndef MY_VERSION
#define MY_VERSION	"1.0"
#endif

#define PROGRAM_NAME	"newick"

#define A(x)		STR_OPT(x)
#include "cmds.h"

#ifndef VECTOR_OPT
#define VECTOR_OPT(x)	/* empty */
#endif

STR_OPT(log)
STR_OPT(tsvout)
STR_OPT(fevout)
STR_OPT(label)
STR_OPT(labels)
STR_OPT(labels2)
STR_OPT(tree)
STR_OPT(tree2)
STR_OPT(report)
STR_OPT(output)
STR_OPT(features)
STR_OPT(nodes)
STR_OPT(cat)
STR_OPT(value)
STR_OPT(edge)
STR_OPT(svg)
STR_OPT(colors)
STR_OPT(legend)
STR_OPT(prefix)
STR_OPT(pattern)
STR_OPT(labelsout)
STR_OPT(default_color)
STR_OPT(title)
STR_OPT(ref)
STR_OPT(trees)
STR_OPT(root_label)
STR_OPT(old_root_label)
STR_OPT(outgroup)
STR_OPT(ff)
STR_OPT(taxtable)
STR_OPT(triangles)
STR_OPT(squares)
STR_OPT(titles)
STR_OPT(labeldx)
STR_OPT(order)
STR_OPT(input2)

UNS_OPT(node,				0,			0,			UINT_MAX)
UNS_OPT(bif,				0,			0,			UINT_MAX)
UNS_OPT(threads,			0,			0,			UINT_MAX)
UNS_OPT(randseed,			0,			0,			UINT_MAX)
UNS_OPT(strokewidth,		0,			0,			UINT_MAX)
UNS_OPT(n,					0,			0,			UINT_MAX)
UNS_OPT(subsetsize,			0,			0,			UINT_MAX)
UNS_OPT(maxpercluster,		0,			0,			UINT_MAX)
UNS_OPT(offsetx,			0,			0,			UINT_MAX)
UNS_OPT(offsety,			0,			0,			UINT_MAX)
UNS_OPT(trees_per_row,		0,			0,			UINT_MAX)
UNS_OPT(tree_width,			0,			0,			UINT_MAX)
UNS_OPT(tree_height,		0,			0,			UINT_MAX)
UNS_OPT(tree_spacing,		0,			0,			UINT_MAX)
UNS_OPT(title_font_size,	0,			0,			UINT_MAX)
UNS_OPT(label_font_size,	0,			0,			UINT_MAX)
UNS_OPT(min_group_size,		0,			0,			UINT_MAX)
UNS_OPT(treeix,				0,			0,			UINT_MAX)
UNS_OPT(nodeix,				0,			0,			UINT_MAX)
UNS_OPT(scalex,				0,			0,			UINT_MAX)
UNS_OPT(scaley,				0,			0,			UINT_MAX)
UNS_OPT(pixelsperunit,		0,			0,			UINT_MAX)
UNS_OPT(kfold,		0,			0,			UINT_MAX)

FLT_OPT(maxdist,			0.2,		0.0,		1.0)
FLT_OPT(minlength,			0.0,		0.0,		FLT_MAX)
FLT_OPT(maxlength,			0.0,		0.0,		FLT_MAX)
FLT_OPT(length,				0.0,		0.0,		FLT_MAX)
FLT_OPT(minconf,			0.0,		0.0,		FLT_MAX)
FLT_OPT(majorityfract,		0.0,		0.0,		FLT_MAX)
FLT_OPT(mintpfract,			0.0,		0.0,		FLT_MAX)
FLT_OPT(minmedleafdist,		0.0,		0.0,		FLT_MAX)
FLT_OPT(maxmedleafdist,		0.0,		0.0,		FLT_MAX)
FLT_OPT(minboot,			0.0,		0.0,		FLT_MAX)

FLAG_OPT(quiet)
FLAG_OPT(log_used_opts)
FLAG_OPT(compilerinfo)
FLAG_OPT(strict_newick)
FLAG_OPT(right)
FLAG_OPT(accs)
FLAG_OPT(trace_parse)
FLAG_OPT(trace_lex)
FLAG_OPT(rooted)
FLAG_OPT(self)
FLAG_OPT(delete_labels)
FLAG_OPT(lcaconfs)
FLAG_OPT(unitlengths)
FLAG_OPT(draw_leaf_labels)
FLAG_OPT(draw_internal_labels)
FLAG_OPT(internal_labels_pct)
FLAG_OPT(title16x)
FLAG_OPT(log_tree)
FLAG_OPT(savetrees)
FLAG_OPT(allow_blank_labels)

VECTOR_OPT(_notused_vector)

#undef FLAG_OPT
#undef UNS_OPT
#undef FLT_OPT
#undef STR_OPT
#undef VECTOR_OPT
