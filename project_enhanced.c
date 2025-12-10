
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//voici les structures
typedef struct noeud{
    int id_util;
    struct noeud* suiv;
} noeud;

typedef struct publication{
    char auteur[10];
    char contenu[50];
    struct publication* suivant;    
} publication;

typedef struct utilisateur{
    int id;
    char nom[10];
    int age;
    char email[20];
    noeud* amis;
    noeud* abons;
    publication* pub;              
    struct utilisateur* gauche;
    struct utilisateur* droite;
} utilisateur;

typedef struct file{
    publication* tete;
    publication* queue;
} file;

//prototypes
//ABR
void ajoutABR(utilisateur **racineABR, utilisateur *util);
utilisateur* rechercheABR(utilisateur *racineABR,int idU);
utilisateur* succ(utilisateur* racineABR);
utilisateur* supprimerABR(utilisateur *racineABR,int idU);

//les_relations
int est_relation(noeud* relations,int idU);
void ajout_relation(noeud **relations ,int id_pr);
void supp_relation(noeud **relations,int id_pr);
int nb_relation(noeud *relations);

//les_publications
void publier(utilisateur *util, char *text);
void undo_publier(utilisateur *util,char* ch);

//timeline(FILE:FIFO)
void timeline_vide(file*timeline);
int est_vide(file*timeline);
void emfiler(file*timeline,publication*pub_recent);
void defiler_afficher(file*timeline);
void creation_timeline(file*timeline,utilisateur*util,utilisateur*racineABR);

//creation d'un utilisateur
utilisateur* cree_util(int idU,const char nomU[10],const char emailU[20],int ageU);
int generer_id();

//interaction avec utilisateur
void menu();
void cree_compte(utilisateur** racineABR);
void afficher_utilisateur(utilisateur* util);
void ajout_ami(utilisateur *racineABR, utilisateur* util);
void supprimer_ami(utilisateur* racineABR, utilisateur* util);
void ajout_abonment(utilisateur *racineABR, utilisateur* util);
void supprimer_abonm(utilisateur* racineABR, utilisateur* util);
void publier_pub(utilisateur* util);
void afficher_timeline(utilisateur* racineABR, utilisateur* util);

//maintenance
utilisateur* charger(FILE* fichier,FILE*fichiertxt);
utilisateur* charger_data(FILE* fichier);
void charger_relations(utilisateur* racineABR, FILE* fichiertxt);
void charger_util_relations(utilisateur* util, FILE* fichiertxt);
void sauvgarder_ABR(utilisateur* racineABR, FILE* fichier, FILE* fichiertxt);
void sauvgarder_util_relations(utilisateur* util, FILE* fichier);
void quitter(utilisateur*racineABR);


//implementation

void ajoutABR(utilisateur **racineABR, utilisateur *util){
    if(!(*racineABR)) {
        *racineABR = util;
    } else if( util->id < (*racineABR)->id ) {
        ajoutABR(&(*racineABR)->gauche, util);
    } else {
        ajoutABR(&(*racineABR)->droite, util);
    }
}

utilisateur* rechercheABR(utilisateur *racineABR,int idU){
    if(!racineABR) return NULL;
    if(racineABR->id == idU) return racineABR;
    if(idU < racineABR->id) return rechercheABR(racineABR->gauche, idU);
    return rechercheABR(racineABR->droite, idU);
}

utilisateur* succ(utilisateur* racineABR){
    if(!racineABR) return NULL;
    if(!racineABR->gauche) return racineABR;
    return succ(racineABR->gauche);
}

utilisateur* supprimerABR(utilisateur *racineABR,int idU){
    if(!racineABR) return NULL;
    if(idU < racineABR->id) racineABR->gauche = supprimerABR(racineABR->gauche, idU);
    else if(idU > racineABR->id) racineABR->droite = supprimerABR(racineABR->droite, idU);
    else {
        utilisateur* temp;
        if(!racineABR->gauche){
            temp = racineABR->droite;
            noeud* p = racineABR->amis;
            while(p){ noeud* q=p->suiv; free(p); p=q; }
            p = racineABR->abons;
            while(p){ noeud* q=p->suiv; free(p); p=q; }
            publication* r = racineABR->pub;
            while(r){ publication* s=r->suivant; free(r); r=s; }
            free(racineABR);
            return temp;
        } else if(!racineABR->droite){
            temp = racineABR->gauche;
            noeud* p = racineABR->amis;
            while(p){ noeud* q=p->suiv; free(p); p=q; }
            p = racineABR->abons;
            while(p){ noeud* q=p->suiv; free(p); p=q; }
            publication* r = racineABR->pub;
            while(r){ publication* s=r->suivant; free(r); r=s; }
            free(racineABR);
            return temp;
        } else {
            temp = succ(racineABR->droite);
            racineABR->id = temp->id;
            strcpy(racineABR->nom, temp->nom);
            strcpy(racineABR->email, temp->email);
            racineABR->age = temp->age;
            racineABR->pub = temp->pub;
            racineABR->abons = temp->abons;
            racineABR->amis = temp->amis;
            racineABR->droite = supprimerABR(racineABR->droite, temp->id);
        }
    }
    return racineABR;
}

//les relations
int est_relation(noeud* relations,int idU){
    noeud* p = relations;
    while(p){
        if(p->id_util == idU) return 1;
        p = p->suiv;
    }
    return 0;
}

void ajout_relation(noeud **relations ,int id_pr){
    noeud* pe = malloc(sizeof(noeud));
    if(!pe){ perror("malloc"); exit(1); }
    pe->id_util = id_pr;
    pe->suiv = *relations;
    *relations = pe;
}

void supp_relation(noeud **relations,int id_pr){
    noeud* p = *relations;
    noeud* pred = NULL;
    while(p && p->id_util != id_pr){
        pred = p;
        p = p->suiv;
    }
    if(p){
        if(!pred) *relations = p->suiv;
        else pred->suiv = p->suiv;
        free(p);
    }
}

int nb_relation(noeud *relations){
    int s = 0;
    noeud* p = relations;
    while(p){ s++; p = p->suiv; }
    return s;
}

//les publications
void publier(utilisateur *util, char *text){
    if(!util) return;
    publication* nouveau = malloc(sizeof(publication));
    if(!nouveau){ perror("malloc"); exit(1); }
    strncpy(nouveau->contenu, text, sizeof(nouveau->contenu)-1);
    nouveau->contenu[sizeof(nouveau->contenu)-1] = '\0';
    strncpy(nouveau->auteur, util->nom, sizeof(nouveau->auteur)-1);
    nouveau->auteur[sizeof(nouveau->auteur)-1] = '\0';
    nouveau->suivant = util->pub;
    util->pub = nouveau;
}

void undo_publier(utilisateur *util,char* ch){
    if(!util || !util->pub) return;
    publication* p = util->pub;
    util->pub = p->suivant;
    strncpy(ch, p->contenu, 49);
    ch[49] = '\0';
    free(p);
}

//timeline
void timeline_vide(file*timeline){
    timeline->queue = NULL;
    timeline->tete = NULL;
}

int est_vide(file*timeline){
    return (timeline->tete == NULL);
}

void emfiler(file*timeline, publication*pub_recent){
    if(!pub_recent) return;
    publication* pr = malloc(sizeof(publication));
    if(!pr){ perror("malloc"); exit(1); }
    strncpy(pr->auteur, pub_recent->auteur, sizeof(pr->auteur)-1); pr->auteur[sizeof(pr->auteur)-1] = '\0';
    strncpy(pr->contenu, pub_recent->contenu, sizeof(pr->contenu)-1); pr->contenu[sizeof(pr->contenu)-1] = '\0';
    pr->suivant = NULL;
    if(est_vide(timeline)) timeline->tete = timeline->queue = pr;
    else {
        timeline->queue->suivant = pr;
        timeline->queue = pr;
    }
}

void defiler_afficher(file*timeline){
    if(est_vide(timeline)) return;
    publication* temp = timeline->tete;
    timeline->tete = temp->suivant;
    if(!timeline->tete) timeline->queue = NULL;
    printf("%s a publie : %s\n", temp->auteur, temp->contenu);
    free(temp);
}

void creation_timeline(file*timeline,utilisateur*util,utilisateur*racineABR){
    timeline_vide(timeline);
    if(!util) return;
    noeud* pe;
    for(pe = util->amis; pe; pe = pe->suiv){
        utilisateur* ami = rechercheABR(racineABR, pe->id_util);
        if(ami) emfiler(timeline, ami->pub);
    }
    for(pe = util->abons; pe; pe = pe->suiv){
        utilisateur* ab = rechercheABR(racineABR, pe->id_util);
        if(ab) emfiler(timeline, ab->pub);
    }
}

//initialiser un utilisateur
utilisateur* cree_util(int idU,const char nomU[10],const char emailU[20],int ageU){
    utilisateur* util = malloc(sizeof(utilisateur));
    if(!util){ perror("malloc"); exit(1); }
    util->id = idU;
    strncpy(util->nom, nomU, sizeof(util->nom)-1); util->nom[sizeof(util->nom)-1] = '\0';
    util->age = ageU;
    strncpy(util->email, emailU, sizeof(util->email)-1); util->email[sizeof(util->email)-1] = '\0';
    util->amis = NULL;
    util->abons = NULL;
    util->pub = NULL;
    util->gauche = util->droite = NULL;
    return util;
}

int generer_id(){
    return rand() + 10000;
}

//menu et les commande
void menu(){
    printf("\n=== MENU ===\n");
    printf("1-afficher_profil\n");
    printf("2-supprimer_compte\n");
    printf("3-ajout_ami\n");
    printf("4-supprimer_ami\n");
    printf("5-nb_de_amis\n");
    printf("6-ajout_abonment\n");
    printf("7-supprimer_abonm\n");
    printf("8-publier_pub\n");
    printf("9-undo_publier\n");
    printf("10-afficher_timeline\n");
    printf("11-quitter\n");
}

void afficher_utilisateur(utilisateur* util){
    if(!util) return;
    printf("\n--- Profil ---\n");
    printf("ID: %d\n", util->id);
    printf("Nom: %s\n", util->nom);
    printf("Age: %d\n", util->age);
    printf("Email: %s\n", util->email);
    printf("Amis: %d  Abonnements: %d" ,nb_relation(util->amis), nb_relation(util->abons));
}

void cree_compte(utilisateur** racineABR){
    char nomU[10], emailU[20];
    int ageU;
    int idU = generer_id();
    printf("Donner ton nom: "); 
    if(!fgets(nomU, sizeof(nomU), stdin)){ nomU[0]=0; }
    nomU[strcspn(nomU, "\n")] = 0;
    printf("Donner ton age: ");
    if(scanf("%d", &ageU) != 1) ageU = 0;
    getchar();
    printf("Donner ton email: ");
    if(!fgets(emailU, sizeof(emailU), stdin)){ emailU[0]=0; }
    emailU[strcspn(emailU, "\n")] = 0;

    utilisateur* util = cree_util(idU, nomU, emailU, ageU);
    ajoutABR(racineABR, util);
    printf("Compte cree ! Ton ID : %d\n", idU);
}

void ajout_ami(utilisateur *racineABR, utilisateur* util){
    int id_ami;
    printf("Donner id de l'ami: "); 
    if(scanf("%d",&id_ami) != 1){ getchar(); printf("Entrée invalide\n"); return; }
    getchar();
    utilisateur* ami = rechercheABR(racineABR, id_ami);
    if(ami){
        if(!est_relation(util->amis, id_ami)){
            ajout_relation(&util->amis, id_ami);
            ajout_relation(&ami->amis, util->id);
            printf("Ami ajoute !\n");
        } else {
            printf("Vous etes deja amis\n");
        }
    } else {
        printf("Ami introuvable\n");
    }
}

void supprimer_ami(utilisateur* racineABR, utilisateur* util){
    int id_ami;
    printf("Donner id de l'ami: ");
    if(scanf("%d",&id_ami) != 1){ getchar(); printf("Entrée invalide\n"); return; }
    getchar();
    utilisateur* ami = rechercheABR(racineABR, id_ami);
    if(ami){
        if(est_relation(util->amis, id_ami)){
            supp_relation(&util->amis, id_ami);
            supp_relation(&ami->amis, util->id);
            printf("Ami supprime !\n");
        } else {
            printf("Vous n'etes pas amis\n");
        }
    } else {
        printf("Ami introuvable\n");
    }
}

void ajout_abonment(utilisateur *racineABR, utilisateur* util){
    int id_abonm;
    printf("Donner id de l'abonne: ");
    if(scanf("%d",&id_abonm) != 1){ getchar(); printf("Entrée invalide\n"); return; }
    getchar();
    utilisateur* abon = rechercheABR(racineABR, id_abonm);
    if(abon){
        if(!est_relation(util->abons, id_abonm)){
            ajout_relation(&util->abons, id_abonm);
            printf("Abonne ajoute !\n");
        } else {
            printf("Vous etes deja abonne\n");
        }
    } else {
        printf("Abonne introuvable\n");
    }
}

void supprimer_abonm(utilisateur* racineABR, utilisateur* util){
    int id_abonm;
    printf("Donner id de l'abonne: ");
    if(scanf("%d",&id_abonm) != 1){ getchar(); printf("Entrée invalide\n"); return; }
    getchar();
    utilisateur* abon = rechercheABR(racineABR, id_abonm);
    if(abon){
        if(est_relation(util->abons, id_abonm)){
            supp_relation(&util->abons, id_abonm);
            printf("Abonne supprime !\n");
        } else {
            printf("Vous n'etes pas abonne\n");
        }
    } else {
        printf("Abonne introuvable\n");
    }
}

void publier_pub(utilisateur* util){
    char txt[50];
    printf("Ecrire: ");
    if(!fgets(txt, sizeof(txt), stdin)) txt[0]=0;
    txt[strcspn(txt, "\n")] = 0;
    publier(util, txt);
    printf("Publication ajoutee !\n");
}

void afficher_timeline(utilisateur* racineABR, utilisateur* util){
    file timeline;
    creation_timeline(&timeline, util, racineABR);
    if(est_vide(&timeline)) {
        printf("Timeline vide.\n");
        return;
    }
    while(!est_vide(&timeline)) defiler_afficher(&timeline);
}


int collecter_publications(publication* sommet, char arr[][50], int max){

    int n = 0;
    publication* p = sommet;
    while(p && n < max){
        strncpy(arr[n], p->contenu, 49); arr[n][49]=0;
        n++;
        p = p->suivant;
    }
    return n;
}

//sauvgarder les relation pour un seul utilisateur
void sauvgarder_util_relations(utilisateur* util, FILE* fichier){
    if(!util || !fichier) return;
    fprintf(fichier, "%d\n", util->id);
    int n_amis = nb_relation(util->amis);
    fprintf(fichier, "%d\n", n_amis);
    noeud* p = util->amis;
    while(p){ fprintf(fichier, "%d\n", p->id_util); p = p->suiv; }
    int n_abons = nb_relation(util->abons);
    fprintf(fichier, "%d\n", n_abons);
    p = util->abons;
    while(p){ fprintf(fichier, "%d\n", p->id_util); p = p->suiv; }
}

void sauvgarder_ABR(utilisateur* racineABR, FILE* fichier, FILE* fichiertxt){
    if(!fichier || !fichiertxt) return;
    if(!racineABR){
        int n = -1;
        fwrite(&n, sizeof(int), 1, fichier);
        return;
    }
    sauvgarder_util_relations(racineABR, fichiertxt);
    fwrite(&racineABR->id, sizeof(int), 1, fichier);
    fwrite(racineABR->nom, sizeof(racineABR->nom), 1, fichier);
    fwrite(&racineABR->age, sizeof(int), 1, fichier);
    fwrite(racineABR->email, sizeof(racineABR->email), 1, fichier);


    const int MAXP = 1024;
    char pubs[MAXP][50];
    int n = collecter_publications(racineABR->pub, pubs, MAXP);
    fwrite(&n, sizeof(int), 1, fichier);
    for(int i = n-1; i >= 0; --i){
        char buf[50];
        strncpy(buf, pubs[i], 49); buf[49]=0;
        fwrite(buf, sizeof(buf), 1, fichier);
    }
    sauvgarder_ABR(racineABR->gauche, fichier, fichiertxt);
    sauvgarder_ABR(racineABR->droite, fichier, fichiertxt);
}

//charger ABR
utilisateur* charger_data(FILE* fichier){
    if(!fichier) return NULL;
    int idU;
    if(fread(&idU, sizeof(int), 1, fichier) != 1) return NULL;
    if(idU == -1) return NULL;

    char nomU[10];
    int ageU;
    char emailU[20];
    if(fread(nomU, sizeof(nomU), 1, fichier) != 1) return NULL;
    if(fread(&ageU, sizeof(int), 1, fichier) != 1) return NULL;
    if(fread(emailU, sizeof(emailU), 1, fichier) != 1) return NULL;

    utilisateur* util = cree_util(idU, nomU, emailU, ageU);

    // publications
    int nb_pubs = 0;
    if(fread(&nb_pubs, sizeof(int), 1, fichier) != 1) { return util; }
    for(int i=0;i<nb_pubs;i++){
        char pubbuf[50];
        if(fread(pubbuf, sizeof(pubbuf), 1, fichier) != 1) break;
        publier(util, pubbuf); 
    }

    util->gauche = charger_data(fichier);
    util->droite = charger_data(fichier);
    return util;
}

//charger les relations
void charger_util_relations(utilisateur* util, FILE* fichiertxt){
    if(!util || !fichiertxt) return;
    int idU;
    if(fscanf(fichiertxt, "%d", &idU) != 1) return;
    int nb_ami;
    if(fscanf(fichiertxt, "%d", &nb_ami) != 1) return;
    for(int i = 0; i < nb_ami; ++i){
        int aid;
        if(fscanf(fichiertxt, "%d", &aid) != 1) break;
        ajout_relation(&util->amis, aid);
    }
    int nb_abonm;
    if(fscanf(fichiertxt, "%d", &nb_abonm) != 1) return;
    for(int i=0;i<nb_abonm;i++){
        int bid;
        if(fscanf(fichiertxt, "%d", &bid) != 1) break;
        ajout_relation(&util->abons, bid);
    }
}

void charger_relations(utilisateur* racineABR, FILE* fichiertxt){
    if(!racineABR || !fichiertxt) return;
    charger_util_relations(racineABR, fichiertxt);
    charger_relations(racineABR->gauche, fichiertxt);
    charger_relations(racineABR->droite, fichiertxt);
}

utilisateur* charger(FILE* fichier,FILE*fichiertxt){
    if(!fichier) return NULL;
    utilisateur* racine = charger_data(fichier);
    if(fichiertxt) charger_relations(racine, fichiertxt);
    return racine;
}

//saugard total
void quitter(utilisateur* racineABR){
    FILE* fbin = fopen("utilisateurs.bin", "wb");
    FILE* ftxt = fopen("relations.txt", "w");
    if(!fbin || !ftxt){
        printf("Erreur: impossible d'ouvrir fichiers pour sauvegarde.\n");
        if(fbin) fclose(fbin);
        if(ftxt) fclose(ftxt);
        return;
    }
    sauvgarder_ABR(racineABR, fbin, ftxt);
    fclose(fbin);
    fclose(ftxt);
    printf("Sauvegarde terminee.\n");
}



int main(){
    srand((unsigned)time(NULL));
    utilisateur* racineABR = NULL;
    FILE* fbin = fopen("utilisateurs.bin", "rb");
    FILE* ftxt = fopen("relations.txt", "r");
    if(fbin){
        racineABR = charger(fbin, ftxt);
        fclose(fbin);
    }
    if(ftxt) fclose(ftxt);

    int action;
    while(1){
        printf("\n=== Bienvenue ===\n");
        printf("1 - Cree un compte\n2 - Entrer (login)\n3 - Sauvegarder et quitter\nChoisir: ");
        if(scanf("%d",&action) != 1){ 
            printf("Entrée invalide\n"); while(getchar()!='\n'); continue;
        }
        getchar();
        if(action == 1){
            cree_compte(&racineABR);
        } else if(action == 2){
            int idU;
            printf("Donner ton ID: ");
            if(scanf("%d",&idU) != 1){ printf("Entrée invalide\n"); while(getchar()!='\n'); continue; }
            getchar();
            utilisateur* util = rechercheABR(racineABR, idU);
            if(!util){
                printf("Utilisateur introuvable\n");
                continue;
            }
            int action2 = 0;
            while(action2 != 11){
                menu();
                printf("Action: ");
                if(scanf("%d", &action2) != 1){ printf("Entrée invalide\n"); while(getchar()!='\n'); continue; }
                getchar();
                switch(action2){
                    case 1: afficher_utilisateur(util); break;
                    case 2:
                        racineABR = supprimerABR(racineABR, util->id);
                        printf("Compte supprime.\n");
                        action2 = 11;
                        break;
                    case 3: ajout_ami(racineABR, util); break;
                    case 4: supprimer_ami(racineABR, util); break;
                    case 5: printf("Nombre d'amis: %d\n", nb_relation(util->amis)); break;
                    case 6: ajout_abonment(racineABR, util); break;
                    case 7: supprimer_abonm(racineABR, util); break;
                    case 8: publier_pub(util); break;
                    case 9: {
                        char ch[50] = "";
                        undo_publier(util, ch);
                        if(strlen(ch)) printf("Publication retiree: %s\n", ch);
                        else printf("Aucune publication a retirer\n");
                        break;
                    }
                    case 10: afficher_timeline(racineABR, util); break;
                    case 11: printf("Retour...\n"); break;
                    default: printf("Donner un entier entre 1 et 11\n"); break;
                }
            }
        } else if(action == 3){
            quitter(racineABR);
            printf("Au revoir.\n");
            break;
        } else {
            printf("Choix invalide\n");
        }
    }

    return 0;
}
