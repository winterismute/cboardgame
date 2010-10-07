void init_game() {
	int result, i, j;

	if (winwidth == 800 && winheight == 600) {
		skybg = GC_LoadImage("skylow.png", &result);
		if ( result == 0) {
			fprintf(stderr, "Impossibile caricare l'immagine skyb.png!\n");
			exit(-1);
		}
	}
	else if (winwidth == 1024 && winheight == 768) {
		skybg = GC_LoadImage("skyhi.png", &result);
		if ( result == 0) {
			fprintf(stderr, "Impossibile caricare l'immagine skyb.png!\n");
			exit(-1);
		}
	}

	screen = SDL_SetVideoMode(winwidth, winheight, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );
	if(screen==NULL) {
		fprintf(stderr,"Can't set video mode: %s\n",SDL_GetError());
		SDL_Quit();
		//return ;
	}

	SDL_WM_SetCaption("LMOthello - Partita!",NULL);

	casellevuote = nrighecolonne * nrighecolonne;
	
	for (i=0; i<nrighecolonne; i++) {
		for (j=0; j<nrighecolonne; j++) {
			scacchieraPrinc[i][j] = VUOTO;
		}
	}

	int startX = (nrighecolonne / 2) -1;
	int startY = (nrighecolonne / 2) -1;
	scacchieraPrinc[startY][startX] = BIANCO;
	scacchieraPrinc[startY][startX + 1] = NERO;
	scacchieraPrinc[startY + 1][startX] = NERO;
	scacchieraPrinc[startY + 1][startX + 1] = BIANCO;
	numerobianche = 2;
	numeronere = 2;
	turno = 0;
	
	if (skybg == NULL) {
		printf("Inizializzazione sfondo fallita. Esco...\n");
		exit(-1);
	}
	else {
		GC_DrawImage(skybg, 0, 0, winwidth, winheight, screen, 0, 0);
	}

}

