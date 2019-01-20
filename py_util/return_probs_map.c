// return probs from tree visit counts
static PyObject *return_probs_map(PyObject *self, PyObject *args){
	int N_TURNS;

	if(!PyArg_ParseTuple(args, "i", &N_TURNS)) return NULL;
	
	ASSERT(N_TURNS > 0, "N_TURNS must be > 0")

	//////// dbg
	int max_tree_sz = tree_sz[0];
	int max_list_sz = list_sz[0];
	for(int gm = 1; gm < BATCH_SZ; gm++){
		if(max_tree_sz < tree_sz[gm])
			max_tree_sz = tree_sz[gm];
		if(max_list_sz < list_sz[gm])
			max_list_sz = list_sz[gm];
	}
	printf("max tree_sz: %i, list_sz %i\n", max_tree_sz, max_list_sz);
	////////

	///// output
	npy_intp dims[3];
	dims[0] = N_TURNS * N_PLAYERS * BATCH_SZ;
	dims[1] = MAP_SZ_X * MAP_SZ_Y;

	PyObject * probs_map_np = PyArray_SimpleNew(2, dims, NPY_FLOAT);

	float * probs_map = (float *) PyArray_DATA((PyArrayObject*) probs_map_np);

	//////////////////////////////////////
	for(int gm = 0; gm < BATCH_SZ; gm++){

		int TO;
		int t_ind = tree_start[gm]; TO_FRM_T_IND
		DASSERT(0 == tree_player[TO])

		unsigned tree_loc = tree_parent[TO];

		// traverse tree backward, alternating players
		for(int turn = N_TURNS-1; turn >= 0; turn--)   for(char player = 1; player >= 0; player--){
			float * probs_map_cur = &probs_map[turn*N_PLAYERS*BATCH_SZ*MAP_SZ + player*BATCH_SZ*MAP_SZ + gm*MAP_SZ];

			// init
			MAP_LOOP probs_map_cur[loc] = 0;
			int TO, LO;

			// inds
			int t_ind = tree_loc; TO_FRM_T_IND
			int l_ind = tree_list_start[TO]; LO_FRM_L_IND
			int n_valid_mvs = tree_list_sz[TO]; CHK_N_VALID_MVS

			DASSERT(n_valid_mvs >= 1);
			DASSERT(player == tree_player[TO]);

			tree_loc = tree_parent[TO];
			
			// set map, sum visits
			int visit_sum = 0;
			for(int mv_ind = 0; mv_ind < n_valid_mvs; mv_ind++){
				int map_loc = list_valid_mv_inds[LO + mv_ind];

				DASSERT(map_loc >= -1 && map_loc < MAP_SZ);
				if(map_loc == -1) continue;

				probs_map_cur[map_loc] = (float)list_visit_count[LO + mv_ind]; 
				visit_sum += list_visit_count[LO + mv_ind];
			}
				
			//  normalize
			for(int mv_ind = 0; (visit_sum != 0) && (mv_ind < n_valid_mvs); mv_ind++){
				int map_loc = list_valid_mv_inds[LO + mv_ind];

				DASSERT(map_loc >= -1 && map_loc < MAP_SZ);
				if(map_loc == -1) continue;

				probs_map_cur[map_loc] /= (float)visit_sum; 
			}
		} // turn / player loops
	} // gm

	return probs_map_np;
}

