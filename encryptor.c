#include <stdio.h>
#include <stdlib.h>

typedef struct pixel{
    unsigned char b, g, r;
};

unsigned int XORSHIFT32 (unsigned int seed[static 1])
{
    unsigned int r = seed[0];
    r = r ^ r << 13;
    r = r ^ r >> 17;
    r = r ^ r << 5;
    seed[0] = r;
    return r;
}


unsigned int *genereare_permutare(unsigned int lungime,unsigned int seed[])
{
    unsigned int *sigma = (unsigned int *) calloc(lungime, sizeof(unsigned int));
    if(sigma == NULL)
    {
        printf("Eroare la alocarea memoriei pentru permutarea sigma!\n");
        return NULL;
    }
    for(unsigned int k = 0;k < lungime;k++)
        sigma[k] = k;
    int indexRND = 1;
    for(unsigned int k = lungime - 1;k >= 1;k--)
    {
        unsigned int rnd = seed[indexRND];
        indexRND++;
        unsigned int aux;
        rnd = rnd % (k + 1);
        aux = sigma[k];
        sigma[k] = sigma[rnd];
        sigma[rnd] = aux;
    }
    return sigma;
}


extern void criptare_imagine(char *numeImagine, char *numeImagineCriptata, FILE *keye)
{
    FILE *in = fopen(numeImagine, "rb");
    FILE *out = fopen(numeImagineCriptata, "wb");

    if(in == NULL)
    {
        printf("Eroare la deschiderea imaginii din care citesc\n");
        return;
    }

    if(out == NULL)
    {
        printf("Eroare la crearea imaginii criptate!\n");
        return;
    }

    unsigned int SV, R0[1];

    fscanf(keye, "%u %u", &R0[0], &SV); //citesc R0 si SV
    //fscanf(keye, "%u", &SV);
    if(in == NULL || keye == NULL)
    {
        printf("Eroare la deschiderea imaginii la criptare");
        return;
    }
    unsigned int latime_img, inaltime_img;
    unsigned char c;
    while(fread(&c, 1, 1, in))
    {
        fwrite(&c, 1, 1, out);//am copiat toti octetii in imaginea criptata
        fflush(out);
    }

    fseek(in, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, in);
    fread(&inaltime_img, sizeof(unsigned int), 1, in);

    unsigned int *RND = (unsigned int *) calloc(2 * latime_img * inaltime_img + 1, sizeof(unsigned int));

    RND[0] = R0[0];

    for(unsigned int i = 1;i <= 2 * latime_img * inaltime_img;i++)
        RND[i] = XORSHIFT32(R0);                    //creez numerele pseudo-aleatoare R0, R1, ... R[2wh]

    unsigned int *sigma = genereare_permutare(latime_img * inaltime_img, RND);
    struct pixel *lin = incarca_imagine(numeImagine);
    struct pixel *linPrim = (struct pixel *)calloc(latime_img * inaltime_img, sizeof(struct pixel));

    if(linPrim == NULL)
    {
        printf("Eroare la alocarea memoriei la criptare!\n");
        return;
    }

    unsigned int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
    unsigned int k = 0;
    for(int i = 0;i < inaltime_img;i++) //PERMUTAREA PIXELILOR
    {
        for(int j = 0;j < latime_img;j++)
        {
            linPrim[sigma[k]] = lin[k];//am permutat pixelii
            k++;
        }
    }

    int indexRND = latime_img * inaltime_img;
    unsigned int X;
    k = 0;

    for(int i = 0;i < inaltime_img;i++) //SCHIMBAREA VALORILOR PIXELILOR
    {
        for(int j = 0;j < latime_img;j++)
        {
            X = RND[indexRND + k];
            if(k == 0)
            {
                c = (SV << 24) >>24; //primul XOR

                linPrim[k].b = c ^ linPrim[k].b;
                c = (SV << 16) >> 24;
                linPrim[k].g = c ^ linPrim[k].g;
                c = (SV << 8) >> 24;
                linPrim[k].r = c ^ linPrim[k].r;

                c = (X << 24) >> 24; //AL DOILEA XOR
                linPrim[k].b = linPrim[k].b ^ c;
                c = (X << 16) >> 24;
                linPrim[k].g = linPrim[k].g ^ c;
                c = (X << 8) >> 24;
                linPrim[k].r = linPrim[k].r ^ c;
            }
            else{
                struct pixel temporar = linPrim[k - 1]; //PRIMUL XOR
                linPrim[k].b = temporar.b ^ linPrim[k].b;
                linPrim[k].g = temporar.g ^ linPrim[k].g;
                linPrim[k].r = temporar.r ^ linPrim[k].r;

                c = (X << 24) >> 24; //AL DOILEA XOR
                linPrim[k].b = linPrim[k].b ^ c;
                c = (X << 16) >> 24;
                linPrim[k].g = linPrim[k].g ^ c;
                c = (X << 8) >> 24;
                linPrim[k].r = linPrim[k].r ^ c;
            }
            k++;
        }
    }

    fseek(out, 54, SEEK_SET);
    for(int i = inaltime_img - 1;i >= 0;i--)///Salvez pixelii
    {
        for(int j = 0;j < latime_img;j++)
        {
            fwrite(linPrim + (i * latime_img + j), sizeof(struct pixel), 1, out);
            fflush(out);
        }
        fseek(out, padding, SEEK_CUR);
    }
    free(lin);
    free(linPrim);
    free(RND);
    free(sigma);
    fclose(out);
}

