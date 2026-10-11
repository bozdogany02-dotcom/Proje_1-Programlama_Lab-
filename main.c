#include <stdio.h>
#include <stdlib.h>
#include <string.h> // String işlemleri için

// Görev durumlarını sayılarla değil isimlerle takip etmek için
typedef enum {
    HENUZ_GELMEDI = 0,
    BEKLIYOR = 1,
    DEVAM_EDIYOR = 2,
    TAMAMLANDI = 3
} GorevDurumu;

struct Gorev {
    // sabit görev verileri
    int ID;
    int GELIS_ZAMANI;
    int SURE;
    int ONCELIK;
    char GOREV_TURU[40];
    char KONUM[20];
    // değişken veriler
    GorevDurumu durum;       // görevin anlık durumu
    int kalan_sure;          // görevin bitmesine kalan süre
    int ilk_baslama_zamani;  // Bekleme süresi hesabı için
    int bitis_zamani;        // makespan ve sistemde kalma hesabı için
    char guncel_teknisyen[3];// "T1", "T2" gibi bilgileri tutacak (Boşsa "")
};

struct Teknisyen {
    char isim[3];            // "T1", "T2" vb.
    int Durum;               // calisma durmu (0/1)
    char Konum[20];          // en son çalıştığı konum
    int Gorev_ID;            // çalıştığı görevin ID'si (Yoksa 0)
    int Calisma_Suresi;      // o ana kadar toplam çalıştığı süre
    int Gecis_Suresi_Kaldi;  // konum değiştirirse 1 olacak
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
    }
    else {
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

            // eksik alan kontrolü
            if (veri_sayisi != 6) {
                printf("Hata: %d. satir reddedildi (Neden: Eksik veya hatali format).\n", satir_no);
                continue;
            }

            // geliş zamanı kontrolü
            if (gorev_listesi[gorev_sayisi].GELIS_ZAMANI < 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz gelis zamani).\n", satir_no);
                continue;
            }

            // süre kontrolü
            if (gorev_listesi[gorev_sayisi].SURE <= 0) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz sure degeri).\n", satir_no);
                continue;
            }

            // öncelik kontrolü
            if ((gorev_listesi[gorev_sayisi].ONCELIK < 1) || (gorev_listesi[gorev_sayisi].ONCELIK > 5)) {
                printf("Hata: %d. satir reddedildi (Neden: Gecersiz oncelik degeri).\n", satir_no);
                continue;
            }

            // tekrarlanan ID kontrolü
            int id_cakismasi = 0;
            for (int i = 0; i < gorev_sayisi; i++) {
                if (gorev_listesi[i].ID == gorev_listesi[gorev_sayisi].ID) {
                    id_cakismasi = 1;
                    break; // ID zaten varsa aramayı bırak
                }
            }
            if (id_cakismasi == 1) {
                printf("Hata: %d. satir reddedildi (Neden: Yinelenen ID).\n", satir_no);
                continue;
            }

            // kontrollerden geçtiyse başlangıç değerleri ayarlanır
            gorev_listesi[gorev_sayisi].durum = HENUZ_GELMEDI;      //sanal zaman 0 iken bütün görevler "HENUZ_GELMEDİ" durumunda
            gorev_listesi[gorev_sayisi].kalan_sure = gorev_listesi[gorev_sayisi].SURE;
            gorev_listesi[gorev_sayisi].ilk_baslama_zamani = -1; // Zaman 0'dan başladığı için, henüz başlamamış göreve "boş/atanmadı" anlamında -1 verilir.
            gorev_listesi[gorev_sayisi].bitis_zamani = -1;      //Tamamlanmamış görevin bitiş zamanı bulunmayacağı için -1 yapıyoruz.
            strcpy(gorev_listesi[gorev_sayisi].guncel_teknisyen, ""); // teknisyen bilgisi boş

            // Sayacı 1 artır, diğer görevi kaydetmeye hazır hale gel
            gorev_sayisi++;
        }

        fclose(dosya);
        printf("\nDosya okuma tamamlandi! Toplam %d gecerli gorev yuklendi.\n", gorev_sayisi);
    }
    //simulasyon degiskenleri
    int sanal_zaman = 0;
    int secili_algoritma = 0; // 1:Shortest job, 2:Priority, 3:Round Robin, 4:Kendi algoritmam
    int quantum_degeri = 0; //Round Robin için
    int secim;

    //ana menu
    while(1){
        printf("\n-------------------------------------------------------\n");
        printf("GUNCEL ZAMAN: %d\n", sanal_zaman);

        //secili algıritma
        printf("SECILI ALGORITMA: ");
        if(secili_algoritma == 1)printf("Shortest Job\n");
        else if(secili_algoritma == 2)printf("Priority\n");
        else if(secili_algoritma == 3)printf("Round Robin\n");
        else if(secili_algoritma == 4)printf("Kendi Algoritmam\n");
        else printf("Henuz secilmedi\n");
        printf("-------------------------------------------------------\n");

        //menu secenekleri
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

        switch(secim){
            case 1:
                printf("\n--- Algoritma Secimi ---\n");
                printf("1- Shortest Job\n");
                printf("2- Priority\n");
                printf("3- Round Robin\n");
                printf("4- Kendi Algoritmam\n");
                printf("Seciminiz: ");
                scanf("%d", &secili_algoritma);

                if(secili_algoritma == 3){
                    printf("Round Robin icin zaman dilimi (quantum) giriniz:");
                    scanf("%d", &quantum_degeri);
                }

                sanal_zaman = 0; //algoritma secildiginde zaman sıfırlandı
                printf("Algoritma secildi. Zaman sifirlandi.\n");
                break;

            case 2:
    printf("\n--- SISTEMDEKI KAYITLI GOREVLER ---\n");
    printf("ID\tGELIS\tSURE\tONCELIK\t\tTUR\t\tKONUM\n");
    printf("------------------------------------------------------------\n\n");
    for(int i = 0; i < gorev_sayisi; i++) {
        printf("%d\t%d\t%d\t%d\t%-20s\t%s\n",
               gorev_listesi[i].ID,
               gorev_listesi[i].GELIS_ZAMANI,
               gorev_listesi[i].SURE,
               gorev_listesi[i].ONCELIK,
               gorev_listesi[i].GOREV_TURU,
               gorev_listesi[i].KONUM);
    }
    printf("------------------------------------------------------------\n");
    printf("Toplam gecerli gorev sayisi: %d\n", gorev_sayisi);
                break;

            case 3:
                printf("\nGorevler durumuna gore listelenecek...\n");
                break;

            case 4:
                printf("\nID ile sorgulama yapilacak...\n");
                break;

            case 5:
    printf("\n--- TEKNISYEN DURUMLARI (ZAMAN: %d) ---\n", sanal_zaman);
    printf("ISIM\tDURUM\t\tGOREV ID\tKONUM\t\tTOPLAM MESAI\n");
    printf("----------------------------------------------------------------------\n");
    for (int t = 0; t < 4; t++) {
        printf("%s\t", teknikerler[t].isim);
        if (teknikerler[t].Durum == 0) {
            printf("BOSTA\t\t-\t\t-\t\t%d birim\n", teknikerler[t].Calisma_Suresi);
        } else {
            printf("MESGUL\t\t%d\t\t%-10s\t%d birim\n",
                   teknikerler[t].Gorev_ID,
                   teknikerler[t].Konum,
                   teknikerler[t].Calisma_Suresi);
        }
    }
    printf("----------------------------------------------------------------------\n");
                break;

            case 6:
    sanal_zaman++;
    printf("\n=== ZAMAN ILERLEDI ===\n");
    printf("Yeni Sanal Zaman: %d\n", sanal_zaman);

    for (int t = 0; t < 4; t++) {
        if (teknikerler[t].Durum == 1) { // Eğer teknisyen doluysa (çalışıyorsa)
            for (int i = 0; i < gorev_sayisi; i++) {
                if (gorev_listesi[i].ID == teknikerler[t].Gorev_ID) { // Teknisyenin uğraştığı görevi bul
                    gorev_listesi[i].kalan_sure--; // Görevin bitmesine 1 saat daha az kaldı
                    teknikerler[t].Calisma_Suresi++; // Teknisyenin toplam mesaisini artır

                    if (gorev_listesi[i].kalan_sure <= 0) { // Görev tamamen bittiyse
                        gorev_listesi[i].durum = TAMAMLANDI;
                        gorev_listesi[i].bitis_zamani = sanal_zaman;

                        printf(" -> [BASARI]: %s teknisyeni %d ID'li gorevi TAMAMLADI!\n",
                               teknikerler[t].isim, gorev_listesi[i].ID);

                        // Teknisyeni yeni görevler alabilmesi için serbest bırak
                        teknikerler[t].Durum = 0;
                        teknikerler[t].Gorev_ID = 0;
                    }
                    break; // Doğru görevi bulup işledik, bu teknisyen için diğer görevlere bakmaya gerek yok
                }
            }
        }
    }
    // Yeni gelen görevleri kontrol et
    int yeni_gelen_gorev = 0;
    for (int i = 0; i < gorev_sayisi; i++) {
        // Görevin geliş zamanı geldiyse ve durumu hala HENUZ_GELMEDI ise:
        if (gorev_listesi[i].GELIS_ZAMANI <= sanal_zaman && gorev_listesi[i].durum == HENUZ_GELMEDI) {
            gorev_listesi[i].durum = BEKLIYOR; // Artık sistemde bekliyor!
            printf(" -> [BILGI]: %d ID'li '%s' gorevi sisteme giris yapti ve siraya girdi!\n",
                   gorev_listesi[i].ID, gorev_listesi[i].GOREV_TURU);
            yeni_gelen_gorev = 1;
        }
    }

    if (yeni_gelen_gorev == 0) {
        printf(" -> Bu zaman diliminde sisteme yeni bir gorev girisi olmadi.\n");
    }
    // Boş teknisyenlere bekleyen görevleri ata (FCFS temeli)
    for (int t = 0; t < 4; t++) {
        if (teknikerler[t].Durum == 0) { // Teknisyen boşsa
            for (int i = 0; i < gorev_sayisi; i++) {
                if (gorev_listesi[i].durum == BEKLIYOR) { // Bekleyen görev bulundu
                    // Görevi teknisyene ata
                    teknikerler[t].Durum = 1;
                    teknikerler[t].Gorev_ID = gorev_listesi[i].ID;
                    strcpy(teknikerler[t].Konum, gorev_listesi[i].KONUM);

                    // Görev durumunu güncelle
                    gorev_listesi[i].durum = DEVAM_EDIYOR;
                    gorev_listesi[i].ilk_baslama_zamani = sanal_zaman;
                    strcpy(gorev_listesi[i].guncel_teknisyen, teknikerler[t].isim);

                    printf(" -> [ATAMA]: %s teknisyeni, %d ID'li goreve basladi.\n",
                           teknikerler[t].isim, gorev_listesi[i].ID);
                    break; // Teknisyen bir görev aldı, diğer görevlere bakmayı bırak
                }
            }
        }
    }

                break;

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
                return 0; //programi sonlandı

            default:
                printf("Gecersiz secim! Lutfen 1-11 arasi bir deger giriniz.\n");
                break;
        }
    }

    return 0;
}
