
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

//voici les structures
typedef struct noeud{
    int id_util;
    struct noeud* suiv;
} noeud;

typedef struct publication{
    char auteur[10];
    char contenu[50];
    struct publication* suivant;    
}publication;

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
}utilisateur;


typedef struct file{
    publication* tete;
    publication* queue;
}file;

//generer un identifiant unique
int generer_id(){
    srand(time(NULL));
    int n=rand();
    return n;
}
//initialiser un utilisateur

utilisateur* cree_util(int idU,char nomU[10],char emailU[20],int ageU){
    utilisateur*util;
    util->id=idU;
    strcpy(util->nom,nomU);
    util->age=ageU;
    strcpy(util->email,emailU);
    util->abons=NULL;
    util->amis=NULL;
    util->droite=NULL;
    util->gauche=NULL;
    return util;


}

//les fcts de base qu'on va utilser apres dans l'app

//voici les fcts relatif aux ABR(arbre)
void ajoutABR(utilisateur **racineABR, utilisateur *util){
    //ajouter util dans l'ABR
    if((*racineABR)==NULL) (*racineABR)=util;
    else{
        if((*racineABR)->id>util->id){
             ajoutABR(&((*racineABR)->gauche),util);
        }
        else{
             ajoutABR(&((*racineABR)->droite),util);
        }
    }
}


utilisateur* rechercheABR(utilisateur *racineABR,int idU){
    //retoune un pointeur sur un utilisateur si il exist sinon null
    if(racineABR==NULL){
        return NULL;
    }
    else if(racineABR->id==idU){
        return racineABR; 
    }
    else if(racineABR->id>idU){
        return rechercheABR(racineABR->gauche,idU);
    }
    else{
        return rechercheABR(racineABR->droite,idU);
    }
}

utilisateur*succ(utilisateur*racineABR){
    if(racineABR->gauche==NULL)
        return racineABR;
    else{
        return succ(racineABR->gauche);
    }
}


utilisateur* supprimerABR(utilisateur *racineABR,int idU){
    //supprimer de l'ABR util d'idU si il existe
    
    if(racineABR==NULL) 
        return NULL;
    if(racineABR->id>idU)
        racineABR->gauche=supprimerABR(racineABR->gauche,idU);
    else if (racineABR->id<idU)
        racineABR->droite=supprimerABR(racineABR->droite,idU);
    else{utilisateur*temp;
        if(racineABR->gauche==NULL){
            temp=racineABR->droite;
            free(racineABR);
            return temp;
        }
        else if(racineABR->droite==NULL){
            temp=racineABR->gauche;
            free(racineABR);
            return temp;
        }
        else{
            temp=succ(racineABR->droite);
            racineABR->id=temp->id;
            strcpy(racineABR->nom,temp->nom);
            strcpy(racineABR->email,temp->email);
            racineABR->age=temp->age;
            racineABR->pub=temp->pub;
            racineABR->abons=temp->abons;
            racineABR->amis=temp->amis;
            racineABR->droite=supprimerABR(racineABR->droite,temp->id);
        }

    }
    return racineABR;
}


//voici les fcts relatif aux relations(list chaine)

int est_relation(noeud*relations,int idU){
    //retoune 1 si idU exist dans la list chaine relations
    int n=0;
    noeud*p;
    p=relations;
    while(p!=NULL){
        if(p->id_util==idU){
            n=1;
            break;
        }
        p=p->suiv;
    }
    return n;

}

void ajout_relation(noeud **relations ,int id_pr){
    //ajout de id_pr dans la liste chaine relations
    noeud*pe=NULL;
    pe=malloc(sizeof(noeud));
    pe->id_util=id_pr;
    pe->suiv=(*relations);
    (*relations)=pe;
}
void supp_relation(noeud **relations,int id_pr){
    //supprimer id_pr de relations
    noeud*p=*relations;
    noeud*pred=NULL;
    while((p!=NULL)&&(p->id_util!=id_pr)){
        pred=p;
        p=p->suiv;
    }
    if(p!=NULL){
        if(pred==NULL){
            *relations=p->suiv;
        }
        else{
            pred->suiv=p->suiv;
        }
        free(p);
    }
}

int nb_relation(noeud *relations){
    //retourner le nb de relation dans la list chainee relations
    noeud*p=relations;
    int s=0;
    while(p!=NULL){
        s+=1;
        p=p->suiv;
    }
    return s;
}


//voici les fcts relatif a la pile de publication(pile:lifo)

void publier(utilisateur *util, char *text){
    //cree une structure publication et l'ajouter
    publication*nouveau;
    nouveau=malloc(sizeof(publication));
    strcpy(nouveau->contenu,text);
    strcpy(nouveau->auteur,util->nom);
    nouveau->suivant=util->pub;
    util->pub=nouveau;
}
void undo_publier(utilisateur *util,char* ch){
    //supprimer la derniere publication publier pour un utilisateur et la mettre dans ch
    publication*p;
    p=util->pub;
    if(p!=NULL){
        util->pub=p->suivant;
        strcpy(ch,p->contenu);
        free(p);
    }
}

//voici les fcts relatif a la timeline(file:fifo)

void timeline_vide(file*timeline){
    timeline->queue=NULL;
    timeline->tete=NULL;
}
int est_vide(file*timeline){
    if(((timeline->tete)==NULL)&&(timeline->queue==NULL)){
        return 1;
    }
    else{
        return 0;
    }

}

void creation_timeline(file*timeline,utilisateur*util,utilisateur*racineABR){
    timeline_vide(timeline);
    publication*pub_recent;
    utilisateur*ami=NULL;
    utilisateur*abonee=NULL;
    noeud*lst_ami=util->amis;
    noeud*lst_abons=util->abons;
    noeud*pe_ami=lst_ami;
    noeud*pe_abon=lst_abons;
    while(pe_ami!=NULL){
        ami=rechercheABR(racineABR,pe_ami->id_util);
        pub_recent=ami->pub;
        emfiler(timeline,pub_recent);
        pe_ami=pe_ami->suiv;
    }
    while(pe_abon!=NULL){
        abonee=rechercheABR(racineABR,pe_abon->id_util);
        pub_recent=abonee->pub;
        emfiler(timeline,pub_recent);
        pe_abon=pe_abon->suiv;
    }
}


void emfiler(file*timeline,publication*pub_recent){
 publication*temp=timeline->queue;
 publication*pr=NULL;
 pr=malloc(sizeof(publication));
 strcpy(pr->auteur,pub_recent->auteur);
 strcpy(pr->contenu,pub_recent->contenu);
 pr->suivant=NULL;
 if(est_vide(timeline)){
    timeline->tete=pr;
 }   
 else{
    temp->suivant=pr;
 }
 timeline->queue=pr;
}

void defiler_afficher(file*timeline){
    publication*temp=timeline->tete;
    if(!(est_vide(timeline))){
        if(timeline->tete==timeline->queue){
            timeline_vide(timeline);

        }
        else timeline->tete=temp->suivant;
        printf("%s a publier %s",temp->auteur,temp->contenu);
        free(temp);
    }
}

//l'interaction avec utilisateur, chargement et sauvegard de data

int main(){
    FILE*fichier;
    FILE*fichiertxt;
    fichier=fopen("utilisateurs.bin","rb");
    fichiertxt=fopen("relations.txt","r");
    utilisateur*racineABR=charger(fichier,fichiertxt);
    fclose(fichier);
    fclose(fichiertxt);
    char ch[50];
    int action;
    int action2=10;
        printf("1-cree un compte");
        printf("2-entrer");
        scanf(" %d",&action);
        switch(action){
            case 1:cree_compte(racineABR);quitter(racineABR); break;
            case 2:
            int idU;
            utilisateur*util;
            printf("donner ton id");
            scanf(" %d",&idU);
            util=rechercheABR(racineABR,idU);
            if(util){
            menu();
            do{ 
                scanf(" %d",&action2);
                switch(action2){
                    case 1:afficher_utilisateur(util); break;
                    case 2:racineABR=supprimerABR(racineABR,idU); break;
                    case 3:ajout_ami(racineABR,util); break;
                    case 4:supprimer_ami(racineABR,util); break;
                    case 5:printf("%d",nb_relation(util->amis));;break;
                    case 6:ajout_abonment(racineABR,util); break;
                    case 7:supprimer_abonm(racineABR,util); break;
                    case 8:publier_pub(util); break;
                    case 9:undo_publier(util,ch);break;
                    case 10:afficher_timeline(racineABR,util); break;
                    case 11:quitter(racineABR); break;
                    default:printf("donner un entier entre 1 et 11");
                }
                menu();
                
                
            
            }while(action2!=11);
        }
        else{printf("utilisateur introuvable");};break;
        default: printf("choisir 1 ou 2");
        }
}

void menu(){
    printf("BONJOUR, choisir entre:\n");
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



void cree_compte(utilisateur*racineABR){
    int idU;
    char nomU[10];
    char emailU[20];
    int ageU;
    utilisateur*util=NULL;
    idU=generer_id();
    printf("donner ton nom");
    fgets(nomU,10,stdin);
    printf("donner ton age");
    scanf("%d",&ageU);
    printf("donner ton email");
    fgets(emailU,20,stdin);
    util=cree_util(idU,nomU,emailU,ageU);
    ajoutABR(racineABR,util);
    printf("your id keep it secret!! %d",idU);    
}

void afficher_utilisateur(utilisateur*util){
        printf("%s\n",util->nom);
        printf("%s\n",util->age);
        printf("%s/n",util->email);   
}


void ajout_ami(utilisateur *racineABR, utilisateur*util){
        int id_ami;
        printf("donner id de l'ami");
        scanf(" %d",&id_ami);
        utilisateur*ami=rechercheABR(racineABR,id_ami);
        if(ami!=NULL){
            if(est_relation(util,id_ami)==0){
            ajout_relation(&(util->amis),id_ami);
            ajout_relation(&(ami->amis),util->id);
            }
            else{
                printf("vous etes deja amis");
            }
        }
        else{
            printf("ami introuvable");
        }
}

void supprimer_ami(utilisateur*racineABR,utilisateur*util){
        int id_ami;
        utilisateur*ami=rechercheABR(racineABR,id_ami);
   
        printf("donner id de l'ami");
        scanf(" %d",&id_ami);
        if(ami!=NULL){
            if(est_relation(util->amis,id_ami)==1){
            supp_relation(&(util->amis),id_ami);
            supp_relation(&(ami->amis),util->id);
            }
            else{
                printf("vous n'etes pas amis");
            }
        }
        else{
            printf("ami introuvable");
        }

}


void ajout_abonment(utilisateur *racineABR,utilisateur*util){
    int id_abonm;
        printf("donner id de l'ami");
        scanf(" %d",&id_abonm);
        if(rechercheABR(racineABR,id_abonm)){
            if(est_relation(util->abons,id_abonm)==0){
                ajout_relation(&(util->abons),id_abonm);
            }
            else{
                printf("vous etes deja abonnee");
            }
        }
        else{
            printf("abonm introuvable");
        }
}


void supprimer_abonm(utilisateur*racineABR,utilisateur*util){
    int id_abonm;
        printf("donner id de l'ami");
        scanf(" %d",&id_abonm);
        if(rechercheABR(racineABR,id_abonm)){
            if(est_relation(util->abons,id_abonm)==1){
                supp_relation(&(util->abons),id_abonm);
            }
            else{
                printf("vous n'etes pas abonnee");
            }
        }
        else{
            printf("abonm introuvable");
        }
}

void publier_pub(utilisateur*util){
    char txt[50];
    printf("ecrire");
    fgets(txt,50,stdin);
    publier(util,txt);

}

void afficher_timeline(utilisateur*racineABR,utilisateur*util){
    file timeline;
    creation_timeline(&timeline,util,racineABR);
    while(!(est_vide(&timeline))){
        defiler_afficher(&timeline);
    }   
}


//les fcts relatif au chargement de l'arbre
utilisateur* charger(FILE*fichier,FILE*fichiertxt){
    utilisateur*racineABR=NULL;
    racineABR=charger_data(fichier);
    charger_relations(racineABR,fichiertxt);
    return racineABR;
}


utilisateur* charger_data(FILE*fichier){
    utilisateur* racineABR=NULL;
    int idU;
    int ageU;
    char nomU[10];
    char emailU[20];
    char pubU[50];
    int k;
    char cst[50];
    strcpy(cst,"___");
    k=fread(&idU,sizeof(idU),1,fichier);
    if(k!=0){
        if(idU==-1){
            return NULL;
        }
        else{
            fread(nomU,sizeof(nomU),1,fichier);
            fread(&ageU,sizeof(ageU),1,fichier);
            fread(emailU,sizeof(emailU),1,fichier);
            racineABR=cree_util(idU,nomU,emailU,ageU);
            fread(pubU,sizeof(pubU),1,fichier);
            while(strcmp(pubU,cst)!=0){
                publier(racineABR,pubU);
                fread(pubU,sizeof(pubU),1,fichier);
            }
            racineABR=inverser_pub(racineABR);
            racineABR->gauche=charger_data(fichier);
            racineABR->droite=charger_data(fichier);
        }
    }
    return racineABR;
}
//pour garentir l'ordre de la pile de pubication
utilisateur* inverser_pub(utilisateur*util){
    char ch[50];
    utilisateur*temp;
    temp=cree_util(util->id,util->nom,util->email,util->age);
    while(util->pub!=NULL){
        undo_publier(util,ch);
        publier(temp,ch);
    } 
    return temp; 
}


void charger_relations(utilisateur*racineABR,FILE*fichiertxt){
    if (racineABR!=NULL){
        charger_util_relations(racineABR,fichiertxt);
        charger_relations(racineABR->gauche,fichiertxt);
        charger_relations(racineABR->droite,fichiertxt);
    }

}



void charger_util_relations(utilisateur*util,FILE*fichiertxt){
    int idU;
    int nb_ami;
    int nb_abonm;
    fscanf(fichiertxt,"%d",&idU);
    fscanf(fichiertxt,"%d",&nb_ami);
    if(nb_ami!=0){
        int t[nb_ami];
        for(int i=nb_ami-1;i>=0;i--){
            fscanf(fichiertxt,"%d",&t[i]);
        }
        for(int i=0;i<nb_ami;i++){
            ajout_relation(&(util->amis),t[i]);
            
        }
    }
    fscanf(fichiertxt,"%d",&nb_abonm);
    if(nb_abonm!=0){
        int tb[nb_abonm];
        for(int i=nb_abonm-1;i>=0;i--){
            fscanf(fichiertxt,"%d",&tb[i]);
        }
        for(int i=0;i<nb_abonm;i++){
            ajout_relation(&(util->abons),tb[i]);
        }
    }
}


//voici les fcts relatif au sauvegard de l'arbre

void quitter(utilisateur*racineABR){
    FILE*fichier_bin=fopen("utilisateurs.bin","wb");
    FILE*fichier_txt=fopen("relations.txt","w");
    sauvgarder_ABR(racineABR,fichier_bin,fichier_txt);
    fclose(fichier_txt);
    fclose(fichier_bin);   
}


void sauvgarder_ABR(utilisateur*racineABR, FILE*fichier,FILE*fichiertxt){
    int n=-1;
    char ch[50];
    if(racineABR==NULL){
        fwrite(&n,sizeof(int),1,fichier);}
    else{
        sauvgarder_util_relations(racineABR,fichiertxt);
        int idU=racineABR->id;
        int ageU=racineABR->age;
        char emailU[20];
        char nomU[10];
        strcpy(emailU,racineABR->email);
        strcpy(nomU,racineABR->nom);
        fwrite(&idU,sizeof(int),1,fichier);
        fwrite(nomU,sizeof(nomU),1,fichier);
        fwrite(&ageU,sizeof(int),1,fichier);
        fwrite(emailU,sizeof(emailU),1,fichier);
        while(racineABR->pub!=NULL){
            undo_publier(racineABR,ch);
            fwrite(ch,sizeof(ch),1,fichier);
        }
        strcpy(ch,"___");
        fwrite(ch,sizeof(ch),1,fichier);

        sauvgarder_ABR(racineABR->gauche,fichier,fichiertxt);
        sauvgarder_ABR(racineABR->droite,fichier,fichiertxt);

    }
}




void sauvgarder_util_relations(utilisateur*util,FILE*fichier){
    noeud*pa=util->amis;
    noeud*pab=util->abons;
    int nb_ami;
    int nb_abonm;
    int idU=util->id;
    nb_ami=nb_relation(util->amis);
    nb_abonm=nb_relation(util->abons);
    fprintf(fichier,"%d\n",idU);
    fprintf(fichier,"%d\n",nb_ami);
    while(pa!=NULL){
        fprintf(fichier,"%d\n",pa->id_util);
        pa=pa->suiv;
    }
    fprintf(fichier,"%d\n",nb_abonm);
    while(pab!=NULL){
        fprintf(fichier,"%d\n",pab->id_util);
        pab=pab->suiv;    
    }
}

































