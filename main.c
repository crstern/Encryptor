#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct pixel{
    unsigned char b, g, r;
};


typedef struct detectie{
    int i, j;
    int culoare;
    double cor;
};


extern struct pixel * incarca_imagine(char *numeImagine)
{
    unsigned int latime_img;
    unsigned int inaltime_img;
    FILE *img = fopen(numeImagine, "rb");
    if(img == NULL)
    {
        printf("Eroare la deschiderea imaginii");
        return NULL;
    }
    fseek(img, 18, SEEK_SET);
    fread(&latime_img, sizeof(int), 1, img);
    fread(&inaltime_img, sizeof(int), 1, img);

    int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
    //printf("padding = %d \n",padding);

    struct pixel *Lin = (struct pixel *)malloc((latime_img + padding) * inaltime_img * sizeof(struct pixel));
    fseek(img, 54, SEEK_SET);
    for(int i = inaltime_img - 1;i >= 0;i--){
        for(int j = 0;j < latime_img;j++)
        {
            unsigned char aux[3];
            fread(aux, sizeof(unsigned char), 3, img);
            Lin[i * latime_img + j].b = aux[0];
            Lin[i * latime_img + j].g = aux[1];
            Lin[i * latime_img + j].r = aux[2];
        }
        fseek(img, padding, SEEK_CUR);
    }

    fclose(img);
    return Lin;
}

extern void salveaza_imagine(char *numeImagine)
{
    FILE *img = fopen(numeImagine, "rb");
    FILE *imgOUT = fopen("savedImg.bmp", "wb");
    if(img == NULL)
    {
        printf("Eroare la deschiderea imaginii din care citesc\n");
        return;
    }

    if(imgOUT == NULL)
    {
        printf("Eroare la crearea imaginii\n");
        return;
    }
    int latime_img, inaltime_img;
    fseek(img, 18, SEEK_SET);
    fread(&latime_img, sizeof(int), 1, img);
    fread(&inaltime_img, sizeof(int), 1, img);
    fseek(img, 0, SEEK_SET);
    struct pixel *lin = incarca_imagine(numeImagine);


    char c;
    for(;fread(&c, 1, 1, img);)
    {
        fwrite(&c, 1, 1, imgOUT);
        fflush(imgOUT);
    }
    int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
    printf("padding = %d \n",padding);

    fseek(imgOUT, 54, SEEK_SET);
    for(int i = inaltime_img - 1;i >= 0;i--)
    {
        for(int j = 0;j < latime_img;j++)
        {
            fwrite(lin + (i * latime_img + j), sizeof(struct pixel), 1, imgOUT);
        }
        fseek(imgOUT, padding, SEEK_CUR);

    }
    fclose(img);
    fclose(imgOUT);
    free(lin);
}


struct pixel **Creaza_Matrice(FILE *img, int *inaltime, int *latime)
{
    int i, j;
    fseek(img, 18, SEEK_SET);
    fread(latime, sizeof(int), 1, img);
    fread(inaltime, sizeof(int), 1, img);
    struct pixel **MAP = (struct pixel **)calloc(*inaltime, sizeof(struct pixel *));

    if(MAP == NULL)
    {
        printf("Eroare la alocarea memoriei I\n");
        return NULL;
    }

    for(i = 0;i < *inaltime;i++)
    {
        MAP[i] = (struct pixel *)calloc(*latime, sizeof(struct pixel));
        if(MAP[i] == NULL)
        {
            printf("Eroare la alocarea memoriei");
            return NULL;
        }
    }

    int padding;
    if((*latime) % 4 != 0)
        padding = 4 - (3 * (*latime)) % 4;
    else padding = 0;

    fseek(img, 54, SEEK_SET);
    for(i = *inaltime - 1; i >= 0;i--)
    {
        for(j = 0;j < *latime;j++)
        {
            unsigned char aux[3];
            fread(aux, sizeof(unsigned char), 3, img);
            MAP[i][j].b = aux[0];
            MAP[i][j].g = aux[1];
            MAP[i][j].r = aux[2];
        }
        fseek(img, padding, SEEK_CUR);
    }
    return MAP;
}


int comparator(const void *a, const void *b)
{
    double va = ((struct detectie *) a)->cor;
    double vb = ((struct detectie *) b)->cor;
    if(va < vb)
        return 1;
    else if(va > vb)
        return -1;
    return 0;
}


int main()
{
    FILE *in = fopen("date_de_intrare.in", "rt");

    if(in == NULL)
    {
        printf("Nu am fisierul din care citesc datele de intrare!\n");
        return -1;
    }

    char nume_imagine_pt_criptat[25], nume_imagine_criptata[25], nume_secret_key[25];
    char nume_imagine_pt_decriptat[25], nume_imagine_decriptata[25];
    fscanf(in, "%s", nume_imagine_pt_criptat);
    fscanf(in, "%s", nume_imagine_criptata);
    fscanf(in, "%s", nume_secret_key);
    fscanf(in, "%s", nume_imagine_pt_decriptat);
    fscanf(in, "%s", nume_imagine_decriptata);

    unsigned int seed[1] = {0}, i;
    FILE *key = fopen(nume_secret_key, "rt");

    if(key == NULL)
    {
        printf("Nu am fisierul din care citesc cheia secreta! \n");
        return -1;
    }

    criptare_imagine(nume_imagine_pt_criptat, nume_imagine_criptata, key);
    fclose(key);
    printf("Your image is encrypted\n");

    fscanf(in,"%s", nume_secret_key);
    key = fopen(nume_secret_key, "rt");
    decriptare_imagine(nume_imagine_pt_decriptat, nume_imagine_decriptata, key);
    printf("Your image is decrypted\n");
    fclose(key);
    fclose(in);
    return 0;
}
