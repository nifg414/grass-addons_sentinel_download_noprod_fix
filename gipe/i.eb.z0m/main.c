/****************************************************************************
 *
 * MODULE:       i.eb.z0m
 * AUTHOR(S):    Yann Chemin - ychemin@gmail.com
 * PURPOSE:      Calculates the momentum roughness length
 *               as seen in Bastiaanssen (1995) 
 *
 * COPYRIGHT:    (C) 2002-2006 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *   	    	 License (>=v2). Read the file COPYING that comes with GRASS
 *   	    	 for details.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/glocale.h>


double z_0m(double sa_vi);

int main(int argc, char *argv[])
{
	struct Cell_head cellhd; //region+header info
	char *mapset; // mapset name
	int nrows, ncols;
	int row,col;

	int verbose=1;
	int heat=0;//Flag for surf. roughness for heat transport output
	struct GModule *module;
	struct Option *input1, *input2, *output1, *output2;
	
	struct Flag *flag1, *flag2;	
	struct History history; //metadata
	
	/************************************/
	/* FMEO Declarations*****************/
	char *name;   // input raster name
	char *result1, *result2; //output raster name
	//File Descriptors
	int infd_savi;
	int outfd1, outfd2;
	
	char *savi;
	char *z0m, *z0h;	
	double coef_z0h;
	int i=0,j=0;
	
	void *inrast_savi;
	unsigned char *outrast1, *outrast2;
	RASTER_MAP_TYPE data_type_output=DCELL_TYPE;
	RASTER_MAP_TYPE data_type_savi;
	/************************************/
	G_gisinit(argv[0]);

	module = G_define_module();
	module->keywords = _("roughness length, energy balance, SEBAL");
	module->description = _("Momentum roughness length (z0m) and surface roughness for heat transport (z0h) as seen in Pawan (2004)");

	/* Define the different options */
	input1 = G_define_option() ;
	input1->key	   = _("savi");
	input1->type       = TYPE_STRING;
	input1->required   = YES;
	input1->gisprompt  =_("old,cell,raster") ;
	input1->description=_("Name of the SAVI map [-1.0;1.0]");
	input1->answer     =_("savi");

	input2 = G_define_option() ;
	input2->key        =_("coef");
	input2->type       = TYPE_DOUBLE;
	input2->required   = NO;
	input2->gisprompt  =_("old,cell,raster");
	input2->description=_("Value of the converion factor from z0m and z0h (Pawan, 2004, used 0.1)");
	input2->answer     =_("0.1");

	output1 = G_define_option() ;
	output1->key        =_("z0m");
	output1->type       = TYPE_STRING;
	output1->required   = YES;
	output1->gisprompt  =_("new,cell,raster");
	output1->description=_("Name of the output z0m layer");
	output1->answer     =_("z0m");

	output2 = G_define_option() ;
	output2->key        =_("z0h");
	output2->type       = TYPE_STRING;
	output2->required   = NO;
	output2->gisprompt  =_("new,cell,raster");
	output2->description=_("Name of the output z0h layer");
	output2->answer     =_("z0h");
	
	
	flag1 = G_define_flag();
	flag1->key = 'h';
	flag1->description = _("z0h output (You have to input a coef value)");
	
	flag2 = G_define_flag();
	flag2->key = 'q';
	flag2->description = _("Quiet");

	/********************/
	if (G_parser(argc, argv))
		exit (EXIT_FAILURE);

	savi	 	= input1->answer;
	coef_z0h 	= atof(input2->answer);
	
	result1  = output1->answer;
	result2  = output2->answer;
	heat    = flag1->answer;
	verbose = (!flag2->answer);
	/***************************************************/
	mapset = G_find_cell2(savi, "");
	if (mapset == NULL) {
		G_fatal_error(_("cell file [%s] not found"), savi);
	}
	data_type_savi = G_raster_map_type(savi,mapset);
	if ( (infd_savi = G_open_cell_old (savi,mapset)) < 0)
		G_fatal_error (_("Cannot open cell file [%s]"), savi);
	if (G_get_cellhd (savi, mapset, &cellhd) < 0)
		G_fatal_error (_("Cannot read file header of [%s])"), savi);
	inrast_savi = G_allocate_raster_buf(data_type_savi);
	/***************************************************/
	G_debug(3, "number of rows %d",cellhd.rows);
	nrows = G_window_rows();
	ncols = G_window_cols();
	outrast1 = G_allocate_raster_buf(data_type_output);
	if(heat){
		outrast2 = G_allocate_raster_buf(data_type_output);
	}
	/* Create New raster files */
	if ( (outfd1 = G_open_raster_new (result1,data_type_output)) < 0)
		G_fatal_error(_("Could not open <%s>"),result1);
	if(heat){
		if ( (outfd2 = G_open_raster_new (result2,data_type_output)) < 0)
			G_fatal_error(_("Could not open <%s>"),result2);
	}
	/* Process pixels */
	for (row = 0; row < nrows; row++)
	{
		DCELL d;
		DCELL d_savi;
		DCELL d_z0h;
		if(verbose)
			G_percent(row,nrows,2);
//		printf("row = %i/%i\n",row,nrows);
		/* read soil input maps */	
		if(G_get_raster_row(infd_savi,inrast_savi,row,data_type_savi)<0)
			G_fatal_error(_("Could not read from <%s>"),savi);
		/*process the data */
		for (col=0; col < ncols; col++)
		{
			d_savi = ((DCELL *) inrast_savi)[col];
			if(G_is_d_null_value(&d_savi)){
				((DCELL *) outrast1)[col] = -999.99;
				if(heat){
					((DCELL *) outrast2)[col] = -999.99;
				}
			}else {
				/****************************/
				/* calculate z0m	    */
				d = z_0m(d_savi);
				((DCELL *) outrast1)[col] = d;
				if(heat){
					d_z0h = d*coef_z0h;
					((DCELL *) outrast2)[col] = d;
				}
			}
		}
		if (G_put_raster_row (outfd1, outrast1, data_type_output) < 0)
			G_fatal_error(_("Cannot write to output raster file"));
		if(heat){
			if (G_put_raster_row (outfd2, outrast2, data_type_output) < 0)
				G_fatal_error(_("Cannot write to output raster file"));
		}
	}

	G_free (inrast_savi);
	G_close_cell (infd_savi);
	
	G_free (outrast1);
	G_close_cell (outfd1);
	
	if(heat){
		G_free (outrast2);
		G_close_cell (outfd2);
	}

	G_short_history(result1, "raster", &history);
	G_command_history(&history);
	G_write_history(result1,&history);

	if(heat){
		G_short_history(result2, "raster", &history);
		G_command_history(&history);
		G_write_history(result2,&history);
	}
	exit(EXIT_SUCCESS);
}

