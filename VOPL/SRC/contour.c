
#include "vopl.h"


struct mesh_face_;

typedef struct mesh_zone_ {
	
	int node1;
	int node2;
	int node3;
	
	struct mesh_face_ *face1;
	struct mesh_face_ *face2;
	struct mesh_face_ *face3;
	
} mesh_zone;

typedef struct mesh_face_ {

	int node1;
	int node2;
	
	mesh_zone *zone1;
	mesh_zone *zone2;

	int is_used;
	
} mesh_face;

typedef struct vopl_ucdmesh_ {

	mesh_face *faces;
	int nb_faces;
	
	mesh_zone *zones;
	int nb_zones;
	
	/* look up array : lookup_faces[n1][n2] = NULL or face */
	mesh_face ***lookup_faces;
	
	double *data;
	int nb_nodes;
	double **nodes;
	int **mesh;

} vopl_ucdmesh;



typedef struct isoline_ {

	double *x;
	double *y;
	int len;

} isoline;

#define MAX_PATHS_PER_LEVEL 20   /* should be enough */

typedef struct isoline_set_ {
	
	double level;
	
	isoline *xisoline;
	int nb_isolines;
	int nb_isolines_max;
	
} isoline_set;

typedef struct iso_contours_ {
	
	double *levels;
	int nb_levels;
	
	isoline_set *xisoline_set;
	
} isocontours;


static isocontours * isocontours_new(double *levels, int nb_levels);
static void isocontours_free(isocontours *contours);
static void get_pnt_on_face(double level, mesh_face* face, double*x, double *y, vopl_ucdmesh *ucdmesh);
static mesh_face *get_zone_other_face(double level, mesh_face* face, mesh_zone * zone, vopl_ucdmesh *ucdmesh);
static mesh_zone *get_adjacent_zone(mesh_face* face, mesh_zone *zone, vopl_ucdmesh *ucdmesh);
static void build_path_from_face(isoline * xisoline, double level, mesh_face* start_face, vopl_ucdmesh *ucdmesh);
static void isoline_build(isoline*  xisoline, double level, vopl_ucdmesh *ucdmesh, mesh_face **valid_faces);
static mesh_face * pick_valid_face(double level, vopl_ucdmesh *ucdmesh, mesh_face **valid_faces, int border_face);
static mesh_face ** collect_valid_faces_for_level(double level, vopl_ucdmesh *ucdmesh);
static void isoline_set_setup(isoline_set *xisoline_set, vopl_ucdmesh *ucdmesh);

static isocontours* isocontours_new(double *levels, int nb_levels)
{
	isocontours *contours = malloc( sizeof(isocontours) );
	
	contours->nb_levels = nb_levels;
	contours->levels    = levels;
	
	contours->xisoline_set = (isoline_set*) calloc( nb_levels , sizeof(isoline_set));
	
	int i;
	
	for (i = 0; i < contours->nb_levels; i++)
	{
		contours->xisoline_set[i].level = levels[i];
		contours->xisoline_set[i].nb_isolines = 0;
		contours->xisoline_set[i].nb_isolines_max = MAX_PATHS_PER_LEVEL;
		contours->xisoline_set[i].xisoline = NULL;
	}
	
	return contours;
}

static void isocontours_free(isocontours *contours)
{
	int i,k;
	
	for (i = 0; i < contours->nb_levels; i++)
	{
		isoline_set *xisoline_set = &contours->xisoline_set[i];
		
		for (k = 0; k < xisoline_set->nb_isolines; k++)
		{
			isoline *xisoline = &xisoline_set->xisoline[k];
			
			free(xisoline->x);
			free(xisoline->y);
		}
	}
	
	return;
}

static void get_pnt_on_face(double level, mesh_face* face, double*x, double *y, vopl_ucdmesh *ucdmesh)
{
	/**/
	double x1 = ucdmesh->nodes[face->node1][0];
	double y1 = ucdmesh->nodes[face->node1][1];
	
	double x2 = ucdmesh->nodes[face->node2][0];
	double y2 = ucdmesh->nodes[face->node2][1];
	
	double v1 = ucdmesh->data[face->node1];
	double v2 = ucdmesh->data[face->node2];
	
	/*  
	    (x2-x1)/(v2-v1) = (x-x1)/(v-v1)  =>  x = x1  + (v-v1)*( (x2-x1) / (v2-v1))
	 
	 */
	
	if ( v1 < v2 )
	{
		*x = x1  + (level-v1)*( (x2-x1) / (v2-v1));
	   *y = y1  + (level-v1)*( (y2-y1) / (v2-v1));
	}
	else
	if ( v2 < v1 )
	{
		*x = x2  + (level-v2)*( (x1-x2) / (v1-v2));
	   *y = y2  + (level-v2)*( (y1-y2) / (v1-v2));
	}	
	else
	{
		*x = (x1+x2)/2.0;
		*y = (y1+y2)/2.0;
	}

	/* mark dace as used */
	face->is_used = 1;
}

static mesh_face *get_zone_other_face(double level, mesh_face* face, mesh_zone * zone, vopl_ucdmesh *ucdmesh)
{	
	if ( zone->face1 == face )
	{
		/* right face is face2 or face3 */
		double node21_value = ucdmesh->data[zone->face2->node1];
		double node22_value = ucdmesh->data[zone->face2->node2];
		
		if ( (node21_value <= level && level <= node22_value) ||  (node22_value <= level && level <= node21_value) )
		{
			if (zone->face2->is_used)
				return NULL;
				
			return zone->face2;
		}
		
		double node31_value = ucdmesh->data[zone->face3->node1];
		double node32_value = ucdmesh->data[zone->face3->node2];
		
		if ( (node31_value <= level && level <= node32_value) || (node32_value <= level && level <= node31_value) )
		{
			if (zone->face3->is_used)
				return NULL;
				
			return zone->face3;
		}
		
		return NULL;
	}
	
	if ( zone->face2 == face )
	{
		/* right face is face2 or face3 */
		double node11_value = ucdmesh->data[zone->face1->node1];
		double node12_value = ucdmesh->data[zone->face1->node2];
		
		if ( (node11_value <= level && level <= node12_value) || (node12_value <= level && level <= node11_value) )
		{
			if (zone->face1->is_used)
				return NULL;
				
			return zone->face1;
		}
		
		double node31_value = ucdmesh->data[zone->face3->node1];
		double node32_value = ucdmesh->data[zone->face3->node2];
		
		if ( (node31_value <= level && level <= node32_value) || (node32_value <= level && level <= node31_value) )
		{
			if (zone->face3->is_used)
				return NULL;
				
			return zone->face3;
		}
		
		return NULL;
	}
	
	if ( zone->face3 == face )
	{
		/* right face is face2 or face3 */
		double node11_value = ucdmesh->data[zone->face1->node1];
		double node12_value = ucdmesh->data[zone->face1->node2];
		
		if ( (node11_value <= level && level <= node12_value) || (node12_value <= level && level <= node11_value) )
		{
			if (zone->face1->is_used)
				return NULL;
				
			return zone->face1;
		}
		
		double node21_value = ucdmesh->data[zone->face2->node1];
		double node22_value = ucdmesh->data[zone->face2->node2];
		
		if ( (node21_value <= level && level <= node22_value) || (node22_value <= level && level <= node21_value) )
		{
			if (zone->face2->is_used)
				return NULL;
				
			return zone->face2;
		}
		
		return NULL;
	}
	
	return NULL;
}

static mesh_zone *get_adjacent_zone(mesh_face* face, mesh_zone *zone, vopl_ucdmesh *ucdmesh)
{
	/* get a zone having as face */
	if (zone == face->zone1 )
	{
		return face->zone2;
	}
	
	if (zone == face->zone2 )
	{
		return face->zone1;
	}
	
	return NULL;
}

static void build_path_from_face(isoline * xisoline, double level, mesh_face* start_face, vopl_ucdmesh *ucdmesh)
{
	mesh_zone *zone = start_face->zone1;
	mesh_face *face = start_face;
	
	xisoline->x = malloc( sizeof(double) * ucdmesh->nb_faces ); /* to big yet */
	xisoline->y = malloc( sizeof(double) * ucdmesh->nb_faces ); /* to big yet */
   xisoline->len = 0;
	
	double x1;
	double y1;
	
	get_pnt_on_face(level, face, &x1, &y1, ucdmesh);
	
	xisoline->x[xisoline->len  ] = x1;
	xisoline->y[xisoline->len++] = y1;
	
	/*printf(" %lf - %lf for face nodes %d & %d  (%lf & %lf)   for zone nodes %d & %d & %d \n", x1, y1, face->node1, face->node2,
			 ucdmesh->data[face->node1], ucdmesh->data[face->node2],
			 zone->node1, zone->node2, zone->node3);
	*/
	while (zone)
	{
		mesh_face *other_face = get_zone_other_face(level, face, zone, ucdmesh);
		
		if ( other_face == NULL )
		{
			if ( face != start_face && start_face->zone2 != NULL ) /* last point to close the curve if start face was "interior face" */
			{
				get_pnt_on_face(level, start_face, &x1, &y1, ucdmesh);
				
				xisoline->x[xisoline->len  ] = x1;
				xisoline->y[xisoline->len++] = y1;
			}
			
			break;
		}
		
		get_pnt_on_face(level, other_face, &x1, &y1, ucdmesh);
		
		xisoline->x[xisoline->len  ] = x1;
		xisoline->y[xisoline->len++] = y1;
		
		/*printf(" %lf - %lf for face nodes %d & %d  (%lf & %lf)   for zone nodes %d & %d & %d \n", x1, y1, other_face->node1, other_face->node2,
				 ucdmesh->data[other_face->node1], ucdmesh->data[other_face->node2],
				 zone->node1, zone->node2, zone->node3);
		*/
		mesh_zone *other_zone = get_adjacent_zone(other_face, zone, ucdmesh);
		
		if ( other_zone == NULL )
		{
			break;
		}
		
		/*printf(" adjacent zone : nodes %d & %d & %d \n",  other_zone->node1, other_zone->node2, other_zone->node3);*/
		
		zone = other_zone;
		face = other_face;
		
		if ( face == start_face )
		{
			break;
		}
	}
}

static mesh_face * pick_valid_face(double level, vopl_ucdmesh* ucdmesh, mesh_face **valid_faces, int border_face)
{
	int i;
	
	for (i=0; i<ucdmesh->nb_faces; i++)
	{
		mesh_face* face = valid_faces[i];
		
		if ( face == NULL )
			break;
		
		if ( face->is_used )
			continue;
		
		if ( border_face )
		{
			if ( face->zone1 != NULL && face->zone2 != NULL )
			    continue; /* not a border face */
			
			return face;
		}
		else 
		{
			if ( face->zone1 == NULL && face->zone2 == NULL )
				continue;
			
			return face;
		}

	}
	
	return NULL;
}

static void isoline_build(isoline* xisoline, double level, vopl_ucdmesh *ucdmesh, mesh_face **valid_faces)
{	
	/* try start with a border face */
	mesh_face *border_start_face   = pick_valid_face(level, ucdmesh, valid_faces, 1);
	mesh_face *interior_start_face = pick_valid_face(level, ucdmesh, valid_faces, 0);
	
	if ( border_start_face )
	{
		build_path_from_face(xisoline, level, border_start_face, ucdmesh);
	}
	else
	if ( interior_start_face )
	{
		build_path_from_face(xisoline, level, interior_start_face, ucdmesh);
	}
	else
	{
		return;
	}
	
	return;
}

static mesh_face ** collect_valid_faces_for_level(double level, vopl_ucdmesh *ucdmesh)
{
	mesh_face **valid_faces = calloc( ucdmesh->nb_faces , sizeof(mesh_face*)); /* too big but Ok */
   
	int f_count = 0;
	int k;
	
	for (k= 0; k<ucdmesh->nb_faces; k++)
	{
		ucdmesh->faces[k].is_used = 0; /* reset */
	}
	
	/* get nb of faces such that level  is on a face */
	for (k = 0; k < ucdmesh->nb_faces; k++)
	{
		mesh_face *face = &ucdmesh->faces[k];
		
		double node1_levels = ucdmesh->data[face->node1];
		double node2_levels = ucdmesh->data[face->node2];
		
		if ( (node1_levels <= level && level <= node2_levels) || (node2_levels <= level && level <= node1_levels) )
		{
			valid_faces[f_count++] = face;
		}
	}
	
	return valid_faces;
}

static void isoline_set_setup(isoline_set *xisoline_set, vopl_ucdmesh *ucdmesh)
{
	double level = xisoline_set->level;
	xisoline_set->nb_isolines = 0; /* init */
	xisoline_set->xisoline = (isoline*)calloc(xisoline_set->nb_isolines_max, sizeof(isoline)); /* 10 max yet ... for a level */
	
	mesh_face **valid_faces = collect_valid_faces_for_level(level, ucdmesh);
	
	int nb = 0;
	/* ok , we 've got the faces where the isolines will pass throught. now build the paths */
	while (1)
	{
		isoline *xisoline = &xisoline_set->xisoline[nb];
		
		/* build a new isoline for this isoline_set */
		isoline_build(xisoline, level, ucdmesh, valid_faces);
		
		if ( xisoline->len == 0 )
		{
			break;
		}
		
		if ( xisoline->len == 1 )
		{
			free(xisoline->x); xisoline->x = NULL;
			free(xisoline->y); xisoline->y = NULL;
			xisoline->len = 0;
		   continue; 	
		}
		nb++;
		
		if ( nb == (xisoline_set->nb_isolines_max - 1) )
		{
			break;
		}
	}
	xisoline_set->nb_isolines = nb;
	
	free(valid_faces);
}

void vopl_ucdmesh_free(void *xucdmesh)
{
	vopl_ucdmesh *ucdmesh = xucdmesh;
	
	free(ucdmesh->zones);
	free(ucdmesh->faces);
	
	int r1;

	
	for (r1=0; r1<ucdmesh->nb_nodes; r1++)
	{
		/* not this!
		 
      int r2;
       
      for (r2=0; r2<ucdmesh->nb_nodes; r2++)
		{
			if (ucdmesh->lookup_faces[r1][r2])
				free(ucdmesh->lookup_faces[r1][r2]);
		}
		*/
		
		if (ucdmesh->lookup_faces[r1] != NULL)
			free(ucdmesh->lookup_faces[r1]);
	}
	
	free(ucdmesh->lookup_faces);
}

void vopl_ucdmesh_setdata(void *xucdmesh, double *data)
{
	vopl_ucdmesh *ucdmesh = xucdmesh;
	
	ucdmesh->data = data;
}	
	
void* vopl_ucdmesh_build(double **nodes, int nb_nodes, int **mesh, int nb_zones)
{
	vopl_ucdmesh *ucdmesh = malloc( sizeof(vopl_ucdmesh) );
	
	ucdmesh->data = NULL; /* to be set with "vopl_ucdmesh_setdata" */
	ucdmesh->nodes = nodes;
	ucdmesh->mesh = mesh;
	
	ucdmesh->nb_nodes = nb_nodes;
	ucdmesh->nb_zones = nb_zones;
	
	ucdmesh->nb_faces = 0;
	ucdmesh->faces = calloc( 3*nb_zones , sizeof(mesh_face)); /* infact less than that - this is the upper limit */
	
	ucdmesh->nb_zones = nb_zones;
	ucdmesh->zones = calloc( nb_zones , sizeof(mesh_zone));
	
	int r1,r2; /* used to see if a face is already defined or not */
	ucdmesh->lookup_faces = (mesh_face ***)calloc(nb_nodes, sizeof(mesh_face **));
	for (r1=0; r1<nb_nodes; r1++)
	{
		ucdmesh->lookup_faces[r1] = (mesh_face **)calloc(nb_nodes, sizeof(mesh_face*));
		
		for (r2=0; r2<nb_nodes; r2++)
			ucdmesh->lookup_faces[r1][r2] = NULL;
	}
		
	int e;
	/* collect all faces in the mesh - avoid duplicated faces - */
	/* setup the allocated "mesh_faces" in the list "faces" */
	/* and order the nodes n the faces (node1 & node2) from the data levels */
	for (e=0; e<nb_zones; e++)
	{
		mesh_zone* zone = &ucdmesh->zones[e];
		
		zone->node1 = mesh[e][0];
		zone->node2 = mesh[e][1];
		zone->node3 = mesh[e][2];
		
		if ( zone->node1 >= nb_nodes )
		{
			printf("    vopl_ucdmesh_build: error nodes id %d >= max node id %d \n", zone->node1, nb_nodes);
		}
		if ( zone->node2 >= nb_nodes )
		{
			printf("    vopl_ucdmesh_build: error nodes id %d >= max node id %d \n", zone->node2, nb_nodes);
		}
		if ( zone->node3 >= nb_nodes )
		{
			printf("    vopl_ucdmesh_build: error nodes id %d >= max node id %d \n", zone->node3, nb_nodes);
		}
			
			
		mesh_face *face1 = &ucdmesh->faces[ucdmesh->nb_faces]; /* a "non-initialized" face */
		
		/*if ( data[zone->node2] < data[zone->node3])
		{
			face1->node1 = zone->node2;
			face1->node2 = zone->node3;
		}
		else
		{
			face1->node1 = zone->node3;
			face1->node2 = zone->node2;
		}*/
		
		face1->node1 = zone->node2;
		face1->node2 = zone->node3;

		mesh_face *xface1 = ucdmesh->lookup_faces[face1->node1][face1->node2];
		
		if ( xface1 )
		{
			zone->face1 = xface1;
			zone->face1->zone2 = zone;
		}
		else 
		{
			ucdmesh->nb_faces++;
			ucdmesh->lookup_faces[face1->node1][face1->node2] = face1;
			ucdmesh->lookup_faces[face1->node2][face1->node1] = face1;
			
			zone->face1 = face1;
			zone->face1->zone1 = zone;
			zone->face1->zone2 = NULL;
		}
		
		/* ------------------------------------------------------------------ */
		
		mesh_face *face2 = &ucdmesh->faces[ucdmesh->nb_faces]; /* a "non-initialized" face */
		
		/*if ( data[zone->node1] < data[zone->node3] )
		{
			face2->node1 = zone->node1;
		   face2->node2 = zone->node3;
		}
		else 
		{
			face2->node1 = zone->node3;
		   face2->node2 = zone->node1;
		}*/
		
		face2->node1 = zone->node1;
		face2->node2 = zone->node3;
		
		mesh_face *xface2 = ucdmesh->lookup_faces[face2->node1][face2->node2];
		
		if ( xface2 )
		{
			zone->face2 = xface2;
			zone->face2->zone2 = zone;
		}
		else 
		{
			ucdmesh->nb_faces++;
			ucdmesh->lookup_faces[face2->node1][face2->node2] = face2;
			ucdmesh->lookup_faces[face2->node2][face2->node1] = face2;
			
			zone->face2 = face2;
			zone->face2->zone1 = zone;
			zone->face2->zone2 = NULL;
		}
		
		/* ------------------------------------------------------------------ */
		
		mesh_face *face3 = &ucdmesh->faces[ucdmesh->nb_faces]; /* a "non-initialized" face */
		
		/*if ( data[zone->node1] < data[zone->node2] )
		{
			face3->node1 = zone->node1;
	   	face3->node2 = zone->node2;
		}
		else 
		{
			face3->node1 = zone->node2;
	   	face3->node2 = zone->node1;
		}*/
		
		face3->node1 = zone->node1;
		face3->node2 = zone->node2;

		mesh_face *xface3 = ucdmesh->lookup_faces[face3->node1][face3->node2];
		
		if ( xface3 )
		{
			zone->face3 = xface3;
			zone->face3->zone2 = zone;
		}
		else 
		{
			ucdmesh->nb_faces++;
			ucdmesh->lookup_faces[face3->node1][face3->node2] = face3;
			ucdmesh->lookup_faces[face3->node2][face3->node1] = face3;
			
			zone->face3 = face3;
			zone->face3->zone1 = zone;
			zone->face3->zone2 = NULL;
		}
	}
	
	/* dump */
	/*
	for (e=0; e<ucdmesh->nb_zones; e++)
	{
		mesh_zone *zone = ucdmesh->zones[e];
		
		printf("zone[%d] : nodes %d  %d  %d \n", e, zone->node1, zone->node2, zone->node3);
		printf("    face1: nodes %d  %d\n", zone->face1->node1, zone->face1->node2);
		printf("    face2: nodes %d  %d\n", zone->face2->node1, zone->face2->node2);
		printf("    face3: nodes %d  %d\n", zone->face3->node1, zone->face3->node2);
	}
	
	for (f=0; f<ucdmesh->nb_faces; f++)
	{
		mesh_face *face = ucdmesh->faces[f];
		
		printf("face[%d] : nodes %d  %d \n", f, face->node1, face->node2);
		if (face->zone1)
			printf("    zone1: nodes %d  %d  %d\n", face->zone1->node1, face->zone1->node2, face->zone1->node3);
		if (face->zone2)
			printf("    zone2: nodes %d  %d  %d\n", face->zone2->node1, face->zone2->node2, face->zone2->node3);
	}
	*/
	
	
	
	return ucdmesh;
}

/* 
 *  contour_mesh
 *
 *     draw the mesh of the domain
 *
 *     double nodes[nb_nodes][2]:  x,y positions of the nodes of the mesh
 *     double data[nb_nodes]    :  data to contour
 *     mesh[nb_zones][3]    :  triangulation data : for each triangle the indices of the nodes
 *     
 */

void contour_mesh(double **nodes, double *data, int nb_nodes, int **mesh, int nb_zones)
{	
	int e;
	
	for (e=0; e<nb_zones; e++)
	{
		int node1 = mesh[e][0];
		int node2 = mesh[e][1];
		int node3 = mesh[e][2];
		
		double x[4] = { nodes[node1][0], nodes[node2][0], nodes[node3][0], nodes[node1][0] };
		double y[4] = { nodes[node1][1], nodes[node2][1], nodes[node3][1], nodes[node1][1] };
		
		plot2(x, y, 4);
	}
}

/* 
 *  contour_border
 *
 *     draw the border of the domain
 *
 *     double nodes[nb_nodes][2]:  x,y positions of the nodes of the mesh
 *     double data[nb_nodes]    :  data to contour
 *     mesh[nb_zones][3]    :  triangulation data : for each triangle the indices of the nodes
 *     
 */

void contour_border(double **nodes, double *data, int nb_nodes, int **mesh, int nb_zones)
{	
	vopl_ucdmesh *ucdmesh = vopl_ucdmesh_build(nodes, nb_nodes, mesh, nb_zones);
	vopl_ucdmesh_setdata(ucdmesh, data);
	
	int i=0;
	int c = 0;
	
	printf("contour_border :  ********************************* \n");
	
	for (i=0; i<ucdmesh->nb_faces; i++)
	{
		if ( ucdmesh->faces[i].zone2 != NULL )
			continue;
		
		int node1 = ucdmesh->faces[i].node1;
		int node2 = ucdmesh->faces[i].node2;
		
		double x[2] = { ucdmesh->nodes[node1][0], ucdmesh->nodes[node2][0] };
		double y[2] = { ucdmesh->nodes[node1][1], ucdmesh->nodes[node2][1] };
		
		c++;
		/*
		 printf("    face[%d] :  (%lf # %lf) to (%lf # %lf)  (global face %d for nodes %d %d)\n", c, x[0], y[0], x[1], y[1], i, node1, node2);
		*/
		plot2(x, y, 2);
	}
	
	printf("contour_border :  nb_faces = %d \n", c);
	
	vopl_ucdmesh_free(ucdmesh);
}


void isolineset_dump(isoline_set *xisoline_set)
{
	printf("***************************************************************\n");
	printf("isoline set : level %lf  : nb isoline = %d\n", xisoline_set->level, xisoline_set->nb_isolines);
}

/* 
 *  contour
 *
 *     draw the contour isovalue lines on a triangular mesh
 *
 *     int nb_levels: number of isolines levels
 *     double *levels : the levels
 *
 *     double nodes[nb_nodes][2]:  x,y positions of the nodes of the mesh
 *     double data[nb_nodes]    :  data to contour
 *     mesh[nb_zones][3]    :  triangulation data : for each triangle the indices of the nodes
 *     
 */

void contour(double *levels, int nb_levels, double **nodes, double *data, int nb_nodes, int **mesh, int nb_zones)
{	
	
		
	/* helper structures setup at the beginning */
	vopl_ucdmesh *ucdmesh = vopl_ucdmesh_build(nodes, nb_nodes, mesh, nb_zones);
	vopl_ucdmesh_setdata(ucdmesh, data);
		
	contour_with_vopl_ucdmesh(levels, nb_levels, data, ucdmesh);
	
	/* helper structures setup at the beginning */
	vopl_ucdmesh_free(ucdmesh);
	
	return;
}

void contour_with_vopl_ucdmesh(double *levels, int nb_levels, double* data, void *ucdmesh)
{	
	int i,k;
	
	isocontours *contours = isocontours_new(levels, nb_levels);
	
	/* helper structures setup at the beginning */
	vopl_ucdmesh * xucdmesh = ucdmesh;
	vopl_ucdmesh_setdata(xucdmesh, data);
	
	for (i = 0; i < contours->nb_levels; i++)
	{
		isoline_set_setup(&contours->xisoline_set[i], xucdmesh);
	}
	
	for (i = 0; i < contours->nb_levels; i++)
	{
		/* nb isolines for this levels in a isoset */
		isoline_set *xisoline_set = &contours->xisoline_set[i];
		
		/*isolineset_dump(xisoline_set);*/
		
		for (k = 0; k < xisoline_set->nb_isolines; k++)
		{
			isoline *xisoline =  &xisoline_set->xisoline[k];
			
			double *x = xisoline->x;
			double *y = xisoline->y;
			int len  = xisoline->len;
			
			if ( i % 7 == 0 )
				color(RED);
			else
			if ( i % 7 == 1 )
				color(GREEN);
			else
			if ( i % 7 == 2 )
				color(YELLOW);
			else
			if ( i % 7 == 3 )
				color(BLUE);
			else
			if ( i % 7 == 4 )
				color(MAGENTA);
			else
			if ( i % 7 == 5 )
				color(CYAN);
			else
			if ( i % 7 == 6 )
				color(WHITE);
			else
			    break;
			
			/* fit(3); */
			
			plot2(x, y, len);
		}
	}
	
	isocontours_free(contours);
	
	return;
}
