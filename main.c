#include <stdio.h>
#include <stdlib.h>
struct Gorev{
int ID;
int GELIS_ZAMANI;
int SURE;
int ONCELIK;
char GOREV_TURU[40];
char KONUM[20];
};

struct Teknisyen{
char isim[3];
int Durum;
char Konum[20];
int Gorev_ID;
int Calisma_Suresi;
};

int main() {
char dosya_adi[50];
char satir[150];
struct Gorev gorev_listesi[2000];
int gorev_sayisi=0;

struct Teknisyen teknikerler[4]={
    {"T1",0,"",0,0},
    {"T2",0,"",0,0},
    {"T3",0,"",0,0},
    {"T4",0,"",0,0}
};

printf("Lutfen bir dosya adi giriniz: ");
scanf("%s",dosya_adi);

FILE* dosya;
dosya=fopen(dosya_adi,"r");
if(dosya==NULL){
printf("Dosya acma islemi basarisiz oldu.");
}
else{
int satir_no=0;
while(fgets(satir,150,dosya)){
satir_no++;
if((satir[0]=='#')||(satir[0]=='\n')){
continue;
}

int veri_sayisi=sscanf(satir, "%d %d %d %d %s %s",
       &gorev_listesi[gorev_sayisi].ID,
       &gorev_listesi[gorev_sayisi].GELIS_ZAMANI,
       &gorev_listesi[gorev_sayisi].SURE,
       &gorev_listesi[gorev_sayisi].ONCELIK,
       gorev_listesi[gorev_sayisi].GOREV_TURU,
       gorev_listesi[gorev_sayisi].KONUM);


if (veri_sayisi != 6) {
    printf("Hata: %d. satir reddedildi (Neden: Eksik veya hatali format).\n", satir_no);
    continue;
}
if (gorev_listesi[gorev_sayisi].SURE <= 0) {
    printf("Hata: %d. satir reddedildi (Neden: Gecersiz sure degeri).\n", satir_no);
    continue;
}
if ((gorev_listesi[gorev_sayisi].ONCELIK < 1) || (gorev_listesi[gorev_sayisi].ONCELIK > 5)) {
    printf("Hata: %d. satir reddedildi (Neden: Gecersiz oncelik degeri).\n", satir_no);
    continue;
}

gorev_sayisi++;
printf("%s",satir);

}
fclose(dosya);





}

return 0;}
