#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Görev durumlarını sayılarla değil isimlerle takip etmek için
typedef enum {
    HENUZ_GELMEDI = 0,
    BEKLIYOR = 1,
    DEVAM_EDIYOR = 2,
    TAMAMLANDI = 3
} GorevDurumu;

// Durumları ekrana yazdırırken metne çevirmek için yardımcı dizi
const char* durum_metinleri[] = {"Henuz Gelmedi", "Bekliyor", "Devam Ediyor", "Tamamlandi"};

struct Gorev {
    // Sabit görev verileri
    int ID;
    int GELIS_ZAMANI;
    int SURE;
    int ONCELIK;
    char GOREV_TURU[40];
    char KONUM[20];

    // Değişken veriler
    GorevDurumu durum;       // Görevin anlık durumu
    int kalan_sure;          // Görevin bitmesine kalan süre
    int ilk_baslama_zamani;  // Bekleme süresi hesabı için
    int bitis_zamani;        // Makespan ve sistemde kalma hesabı için
    char guncel_teknisyen[3];// "T1", "T2" gibi bilgileri tutacak (Boşsa "")
};

struct Teknisyen {
    char isim[3];            // "T1", "T2" vb.
    int Durum;               // Çalışma durumu (0: Bosta, 1: Mesgul)
    char Konum[20];          // En son çalıştığı konum
    int Gorev_ID;            // Çalıştığı görevin ID'si (Yoksa 0)
    int Calisma_Suresi;      // O ana kadar toplam çalıştığı süre
    int Gecis_Suresi_Kaldi;  // Konum değiştirirse 1 olacak
    int Dilimde_Kalan_Sure;  // Round Robin için, quantum sayacı
};

int main() {
    char dosya_adi[50];
    char satir[150];
    struct Gorev gorev_listesi[2000];
    int gorev_sayisi = 0;

    struct Teknisyen teknikerler[4] = {
        {"T1", 0, "", 0, 0, 0, 0},
        {"T2", 0, "", 0, 0, 0, 0},
        {"T3", 0, "", 0, 0, 0, 0},
        {"T4", 0, "", 0, 0, 0, 0}
    };

    printf("Lutfen bir dosya adi giriniz: ");
    scanf("%s", dosya_adi);

    FILE* dosya = fopen(dosya_adi, "r");

    if (dosya == NULL) {
        printf("Dosya acma islemi basarisiz oldu.\n");
    } else {
        int satir_no = 0;

        while (fgets(satir, 150, dosya)) {
            satir_no++;

            // (#) ve boş satırları atlamak için
            if ((satir[0] == '#') || (satir[0] == '\n')) {
                continue;
            }

            int veri_sayisi = sscanf(satir, "%d %d %d %d %s %s",
                                     &gorev_listesi[gorev_sayisi].ID,
                                     &gorev_listesi[gorev_sayisi].GELIS_ZAMANI,
                                     &gorev_listesi[gorev_sayisi].SURE,
                                     &gorev_listesi[gorev_sayisi].ONCELIK,
                                     gorev_listesi[gorev_sayisi].GOREV_TURU,
                                     gorev_listesi[gorev_sayisi].KONUM);

            // Eksik alan kontrolü
            if (veri_sayisi != 6) {
                printf("Hata: %d. satir reddedildi (Neden: Eksik veya hatali format).\n", satir_no);
                continue;
            }

            // Geliş zamanı kontrolü
            if (gorev_listesi[gorev_sayisi].GELIS_ZAMANI < 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz gelis zamani).\n", satir_no);
                continue;
            }

            // Süre kontrolü
            if (gorev_listesi[gorev_sayisi].SURE <= 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz sure degeri).\n", satir_no);
                continue;
            }

            // Öncelik kontrolü
            if ((gorev_listesi[gorev_sayisi].ONCELIK < 1) || (gorev_listesi[gorev_sayisi].ONCELIK > 5)) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz oncelik degeri).\n", satir_no);
                continue;
            }

            // Tekrarlanan ID kontrolü
            int id_cakismasi = 0;
            for (int i = 0; i < gorev_sayisi; i++) {
                if (gorev_listesi[i].ID == gorev_listesi[gorev_sayisi].ID) {
                    id_cakismasi = 1;
                    break;
                }
            }
            if (id_cakismasi == 1) {
                printf("Hata: %d. satir reddedildi (Neden: Yinelenen ID).\n", satir_no);
                continue;
            }

            // Kontrollerden geçtiyse başlangıç değerleri ayarlanır
            gorev_listesi[gorev_sayisi].durum = HENUZ_GELMEDI;
            gorev_listesi[gorev_sayisi].kalan_sure = gorev_listesi[gorev_sayisi].SURE;
            gorev_listesi[gorev_sayisi].ilk_baslama_zamani = -1;
            gorev_listesi[gorev_sayisi].bitis_zamani = -1;
            strcpy(gorev_listesi[gorev_sayisi].guncel_teknisyen, "");

            gorev_sayisi++;
        }

        fclose(dosya);
        printf("\nDosya okuma tamamlandi! Toplam %d gecerli gorev yuklendi.\n", gorev_sayisi);
    }

    // Simülasyon değişkenleri
    int sanal_zaman = 0;
    int secili_algoritma = 0;
    int quantum_degeri = 0;
    int secim;

    // Ana menü döngüsü
    while(1) {
        printf("\n-------------------------------------------------------\n");
        printf("GUNCEL ZAMAN: %d\n", sanal_zaman);

        printf("SECILI ALGORITMA: ");
        if(secili_algoritma == 1) printf("Shortest Job\n");
        else if(secili_algoritma == 2) printf("Priority\n");
        else if(secili_algoritma == 3) printf("Round Robin (Quantum: %d)\n", quantum_degeri);
        else if(secili_algoritma == 4) printf("Kendi Algoritmam\n");
        else printf("Henuz secilmedi\n");
        printf("-------------------------------------------------------\n");

        printf("1. Algoritma sec veya degistir\n");
        printf("2. Tum gorevleri listele\n");
        printf("3. Gorevleri durumuna gore listele\n");
        printf("4. Gorev ID ile sorgula\n");
        printf("5. Teknisyenlerin durumunu goster\n");
        printf("6. Zamani 1 birim ilerlet\n");
        printf("7. Zamani N birim ilerlet\n");
        printf("8. Belirli bir zamana git\n");
        printf("9. Istatistikleri goster\n");
        printf("10. Simulasyonu sifirla\n");
        printf("11. Cikis\n");
        printf("-------------------------------------------------------\n");
        printf("Seciminiz: ");
        scanf("%d", &secim);

        switch(secim) {
            case 1:
                printf("\n--- Algoritma Secimi ---\n");
                printf("1- Shortest Job\n");
                printf("2- Priority\n");
                printf("3- Round Robin\n");
                printf("4- Kendi Algoritmam\n");
                printf("Seciminiz: ");
                scanf("%d", &secili_algoritma);

                if(secili_algoritma == 3) {
                    printf("Round Robin icin zaman dilimi (quantum) giriniz: ");
                    scanf("%d", &quantum_degeri);
                }

                sanal_zaman = 0;
                printf("Algoritma secildi. Zaman sifirlandi.\n");
                break;

            case 2:
                printf("\n--- SISTEMDEKI KAYITLI GOREVLER ---\n");
                printf("ID\tGELIS\tTOPLAM\tONCELIK\tDURUM\t\tTEKNISYEN\tKALAN\n");
                printf("--------------------------------------------------------------------------------\n");
                for(int i = 0; i < gorev_sayisi; i++) {
                    printf("%d\t%d\t%d\t%d\t%-15s\t%-9s\t%d\n",
                           gorev_listesi[i].ID,
                           gorev_listesi[i].GELIS_ZAMANI,
                           gorev_listesi[i].SURE,
                           gorev_listesi[i].ONCELIK,
                           durum_metinleri[gorev_listesi[i].durum],
                           (strcmp(gorev_listesi[i].guncel_teknisyen, "") == 0) ? "-" : gorev_listesi[i].guncel_teknisyen,
                           gorev_listesi[i].kalan_sure);
                }
                printf("--------------------------------------------------------------------------------\n");
                printf("Toplam gecerli gorev sayisi: %d\n", gorev_sayisi);
                break;

            case 3:
                //her durumdaki görev sayısını tutmak için dizi
                int durum_sayilari[4] = {0, 0, 0, 0}; //{henuz gelmedi, bekliyor, devam ediyor, tamamlandı}

                printf("\n================================================================================\n");
                printf("                     GOREVLERIN DURUMUNA GORE LISTESI\n");
                printf("================================================================================\n");

                for(int d = 0; d<4; d++){ //4 farklı durum için döngü
                    printf("\n>>> %s OLAN GOREVLER <<<\n", durum_metinleri[d]);
                    printf("ID\tGELIS\tTOPLAM\tONCELIK\tDURUM\t\tTEKNISYEN\tKALAN\n");
                    printf("--------------------------------------------------------------------------------\n");

                    int bu_durumdaki_sayi = 0; //sadece o anki başlık için sayaç

                    for(int i=0; i<gorev_sayisi; i++){
                        if(gorev_listesi[i].durum == d){
                            bu_durumdaki_sayi++;
                             printf("%d\t%d\t%d\t%d\t%-15s\t%-9s\t%d\n",
                                    gorev_listesi[i].ID,
                                    gorev_listesi[i].GELIS_ZAMANI,
                                    gorev_listesi[i].SURE,
                                    gorev_listesi[i].ONCELIK,
                                    durum_metinleri[gorev_listesi[i].durum],
                                    (strcmp(gorev_listesi[i].guncel_teknisyen, "") == 0) ? "-" : gorev_listesi[i].guncel_teknisyen,
                                    gorev_listesi[i].kalan_sure);
                        }
                    }
                    durum_sayilari[d] = bu_durumdaki_sayi;

                    if(bu_durumdaki_sayi == 0){
                        printf("- Bu durumda hic gorev bulunmamaktadir -\n");
                    }
                }
                break;

            case 4:
                int aranan_id;
                int bulundu = 0;
                printf("\nSorgulamak istediginiz Gorev ID'sini giriniz: ");
                scanf("%d",&aranan_id); // aranan ID girilir

                for(int i=0; i<gorev_sayisi; i++){
                    if(gorev_listesi[i].ID == aranan_id){
                        bulundu = 1;
                        printf("\n--- GOREV DETAYLARI (ID: %d) ---\n", aranan_id);

                        printf("Gelis Zamani        : %d\n", gorev_listesi[i].GELIS_ZAMANI);
                        printf("Toplam Sure         : %d\n", gorev_listesi[i].SURE);
                        printf("Oncelik             : %d\n", gorev_listesi[i].ONCELIK);
                        printf("Durum               : %s\n", durum_metinleri[gorev_listesi[i].durum]);

                        printf("Guncel Teknisyen    : %s\n", (strcmp(gorev_listesi[i].guncel_teknisyen, "") == 0)? "-" : gorev_listesi[i].guncel_teknisyen);//teknisyen yoksa "-" yaz
                        printf("Kalan Sure          : %d\n", gorev_listesi[i].kalan_sure);

                        printf("Gorev Turu          : %s\n", gorev_listesi[i].GOREV_TURU);
                        printf("Konum               : %s\n", gorev_listesi[i].KONUM);

                        //Başlama zamanı kontrol
                        if(gorev_listesi[i].ilk_baslama_zamani != -1){
                            printf("Ilk Baslama Zamani  : %d\n",gorev_listesi[i].ilk_baslama_zamani);
                        } else{
                            printf("Ilk Baslama Zamani  : - (Henuz baslamadi)\n");
                        }

                        //Bitiş zamanı kontrol
                        if(gorev_listesi[i].bitis_zamani != -1){
                            printf("Bitis Zamani        : %d\n", gorev_listesi[i].bitis_zamani);
                        } else{
                            printf("Bitis Zamani        : - (Tamamlanmadi)\n");
                        }
                        break; //Görev bulundu, dizinin devamının taranmasına gerek yok
                    }
                }
                if(bulundu == 0){
                    printf("\nHata: %d ID'li gorev bulunamadi!\n", aranan_id);
                }
                break;

            case 5:
                printf("\n--- TEKNISYEN DURUMLARI (ZAMAN: %d) ---\n", sanal_zaman);
                printf("ISIM\tDURUM\t\tGOREV ID\tGOREV KALAN\tTOPLAM MESAI\n");
                printf("----------------------------------------------------------------------\n");
                for (int t = 0; t < 4; t++) {
                    printf("%s\t", teknikerler[t].isim);
                    if (teknikerler[t].Durum == 0) {
                        printf("BOSTA\t\t-\t\t-\t\t%d birim\n", teknikerler[t].Calisma_Suresi);
                    } else {
                        // Teknisyenin üstündeki görevin kalan süresini bul
                        int anlik_kalan = 0;
                        for(int i=0; i<gorev_sayisi; i++){
                            if(gorev_listesi[i].ID == teknikerler[t].Gorev_ID){
                                anlik_kalan = gorev_listesi[i].kalan_sure;
                                break;
                            }
                        }
                        printf("MESGUL\t\t%d\t\t%d\t\t%d birim\n",
                               teknikerler[t].Gorev_ID,
                               anlik_kalan,
                               teknikerler[t].Calisma_Suresi);
                    }
                }
                printf("----------------------------------------------------------------------\n");
                break;

            case 6: {
                // algoritma seçilmeden zaman ilerletilemez
                if (secili_algoritma == 0) {
                    printf("\nHata: Lutfen once 1. secenekten bir algoritma seciniz!\n");
                    break;
                }

                // mevcut görevleri ilerletme ve konum cezası kontrolü
                for (int t = 0; t < 4; t++) {
                    if (teknikerler[t].Durum == 1) { // teknisyen doluysa

                        // teknisyenin geçiş cezası varsa bu tur sadece yolda geçer
                        if (teknikerler[t].Gecis_Suresi_Kaldi > 0) {
                            teknikerler[t].Gecis_Suresi_Kaldi--;
                            // görev ilerlemez, sadece yolda vakit kaybeder
                        }
                        else {
                            // ceza yoksa normal çalışma yapılır
                            for (int i = 0; i < gorev_sayisi; i++) {
                                if (gorev_listesi[i].ID == teknikerler[t].Gorev_ID) {
                                    gorev_listesi[i].kalan_sure--;     // görevin süresini 1 azalt
                                    teknikerler[t].Calisma_Suresi++;   // teknisyenin toplam çalışma süresinş 1 artır

                                    // algoritma Round Robin ise kuantum sayacını da azalt
                                    if (secili_algoritma == 3) {
                                        teknikerler[t].Dilimde_Kalan_Sure--;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }

                // sanal zamanı ilerlet
                sanal_zaman++;
                printf("\n=== ZAMAN ILERLEDI ===\n");
                printf("Yeni Sanal Zaman: %d\n", sanal_zaman);

                // biten ve dilimi dolan (round robin) görevleri çıkarma
                for(int t=0; t < 4; t++){
                    if(teknikerler[t].Durum == 1){
                        for(int i=0; i < gorev_sayisi; i++){
                            if(gorev_listesi[i].ID == teknikerler[t].Gorev_ID){ //teknisyenin şuanki görevini bul

                                // a) görev tamamen bittiyse
                                if(gorev_listesi[i].kalan_sure == 0){
                                    gorev_listesi[i].durum = TAMAMLANDI;
                                    gorev_listesi[i].bitis_zamani = sanal_zaman;
                                    printf(" -> [BASARI]: %s teknisyeni %d ID'li gorevi TAMAMLADI!\n",
                                           teknikerler[t].isim, gorev_listesi[i].ID);

                                    teknikerler[t].Durum = 0; //teknisyen boşa çıkarılır
                                    teknikerler[t].Gorev_ID = 0;
                                }
                                // b) görev bitmedi ama quantum dolduysa
                                else if(secili_algoritma == 3 && teknikerler[t].Dilimde_Kalan_Sure == 0){
                                    gorev_listesi[i].durum = BEKLIYOR;
                                    printf(" -> [ZAMAN DOLDU]: %d ID'li gorev %s teknisyeninden zorla alindi ve kuyruga atildi.\n",
                                           gorev_listesi[i].ID, teknikerler[t].isim);

                                    teknikerler[t].Durum = 0; //teknisyen boşa çıkarılır
                                    teknikerler[t].Gorev_ID = 0;
                                }

                                break; // görevi bulundu, diğerlerine bakmaya gerek yok
                            }
                        }
                    }
                }

                // yeni gelen görevleri kuyruğa ekleme
                int yeni_gelen_gorev = 0;
                for (int i=0; i<gorev_sayisi; i++) {
                    if (gorev_listesi[i].durum == HENUZ_GELMEDI && gorev_listesi[i].GELIS_ZAMANI <= sanal_zaman) {
                        gorev_listesi[i].durum = BEKLIYOR;
                        printf(" -> [SISTEME GIRDI]: %d ID'li gorev bekleme kuyruguna katildi.\n", gorev_listesi[i].ID);
                        yeni_gelen_gorev = 1;
                    }
                }

                if (yeni_gelen_gorev == 0) {
                    printf(" -> Bu turda sisteme yeni bir gorev girisi olmadi.\n");
                }

                // eksik
                // ATAMA İŞLEMLERİ BURAYA GELECEK
                // if (secili_algoritma == 1) Shortest_Job_Ata();
                // else if (secili_algoritma == 2) Priority_Ata(); ...

                break;
            }

            case 7:
                printf("\nZaman N birim ilerletilecek...\n");
                break;

            case 8:
                printf("\nBelirli bir zamana gidilecek...\n");
                break;

            case 9:
                printf("\nIstatistikler gosterilecek...\n");
                break;

            case 10:
                printf("\nSimulasyon sifirlanacak...\n");
                break;

            case 11:
                printf("\nProgramdan cikiliyor...\n");
                return 0;

            default:
                printf("Gecersiz secim! Lutfen 1-11 arasi bir deger giriniz.\n");
                break;
        }
    }

    return 0;
}
