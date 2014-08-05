
/****************************************************************
 *
 * MODULE:     i.pr
 *
 * AUTHOR(S):  Stefano Merler, ITC-irst
 *
 * PURPOSE:    i.pr - Pattern Recognition
 *
 * COPYRIGHT:  (C) 2007 by the GRASS Development Team
 *
 *             This program is free software under the
 *             GNU General Public License (>=v2).
 *             Read the file COPYING that comes with GRASS
 *             for details.
 *
 ****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include "global.h"

#define MAXLIMITS 20

int read_selection();

int main(argc, argv)
     int argc;
     char **argv;
{
    struct GModule *module;
    struct Option *opt1;
    struct Option *opt2;
    struct Option *opt3;
    struct Option *opt4;

    int i, j, k;
    Features features;

    char features_out_name[500];
    int limits[MAXLIMITS];
    int *selection;
    char *tmpbuf;
    int nselection;
    int nlimits;
    double **copydata;
    int col;

    /* Initialize the GIS calls */
    G_gisinit(argv[0]);

    module = G_define_module();
    module->keywords = _("imagery, image processing, pattern recognition");
    module->description =
	_("Module for feature extraction. "
	  "i.pr: Pattern Recognition environment for image processing. Includes kNN, "
	  "Decision Tree and SVM classification techniques. Also includes "
	  "cross-validation and bagging methods for model validation.");

    /* set up command line */
    opt1 = G_define_option();
    opt1->key = "features";
    opt1->type = TYPE_STRING;
    opt1->required = YES;
    opt1->description =
	"Input file containing the features (output of i.pr_features).";

    opt2 = G_define_option();
    opt2->key = "selected";
    opt2->type = TYPE_STRING;
    opt2->required = YES;
    opt2->description =
	"File containing the results of the features selection procedure\n\t\t(output of i.pr_features_selection).";


    opt3 = G_define_option();
    opt3->key = "nvar";
    opt3->type = TYPE_INTEGER;
    opt3->required = YES;
    opt3->multiple = YES;
    opt3->description = "Number of reordered variables to be extracted.";


    opt4 = G_define_option();
    opt4->key = "output";
    opt4->type = TYPE_STRING;
    opt4->required = NO;
    opt4->description =
	"Optionally, creates output features files with output option as root\n\t\t(insteed of the features option).";


    if (G_parser(argc, argv))
	exit(1);

    nlimits = 0;
    /*get limits */
    if (opt3->answers) {
	j = 0;
	for (i = 0; (tmpbuf = opt3->answers[i]); i++, nlimits++) {
	    if (i == MAXLIMITS)
		break;
	    sscanf(tmpbuf, "%d", &(limits[i]));
	    j += 1;
	}
    }

    /*read features */
    read_features(opt1->answer, &features, -1);

    /*copy data */
    copydata = (double **)G_calloc(features.nexamples, sizeof(double *));
    for (i = 0; i < features.nexamples; i++)
	copydata[i] =
	    (double *)G_calloc(features.examples_dim, sizeof(double));

    for (i = 0; i < features.nexamples; i++)
	for (j = 0; j < features.examples_dim; j++)
	    copydata[i][j] = features.value[i][j];

    /*read relative importance file */
    nselection = read_selection(opt2->answer, &selection);

    if (nselection != features.examples_dim) {
	fprintf(stderr,
		"WARNING: number of features (=%d) different from length of relative importance file (=%d)\n",
		features.examples_dim, nselection);
    }

    /*build files */
    for (i = 0; i < nlimits; i++) {
	features.training.file = "generated by i.pr_features_extract";
	features.training.cols = limits[i];
	features.examples_dim = limits[i];

	col = 0;
	for (j = 0; j < limits[i]; j++) {
	    for (k = 0; k < features.nexamples; k++)
		features.value[k][col] = copydata[k][selection[j] - 1];
	    col++;
	}

	/*write features */
	if (opt4->answer == NULL)
	    sprintf(features_out_name, "%s_fsExtr_%d", opt1->answer,
		    limits[i]);
	else
	    sprintf(features_out_name, "%s_fsExtr_%d", opt4->answer,
		    limits[i]);

	write_features(features_out_name, &features);
    }

    return 0;
}

int read_selection(file, selection)
     char *file;
     int **selection;
{
    FILE *fp;
    char tmpbuf[500];
    char *line = NULL;
    int index = 0;

    if ((fp = fopen(file, "r")) == NULL) {
	sprintf(tmpbuf, "Error opening file %s for reading", file);
	G_fatal_error(tmpbuf);
    }

    *selection = (int *)calloc(1, sizeof(int));
    while ((line = GetLine(fp)) != NULL) {
	line = (char *)strchr(line, '\t');
	sscanf(line, "%d", &((*selection)[index]));
	index++;
	*selection = realloc(*selection, (index + 1) * sizeof(int));
    }

    return index;
}
