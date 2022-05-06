### important concepts from the [2021 publication](./midifileperformer.pdf)

NB : The "delta over equality" sign (&#x225C;) used extensively through the publication means "is equal by definition to"

* Principles of the algorithm that creates the Chronology from Events :
    * 1) First we group events (note on / note off) into event sets by same date
    * 2) Then we redistribute events in consecutive sets so that the Chronology becomes an alternation of starting event sets and ending event sets
    * 3) Then, optionally, we redistribute events one more time to avoid (when possible) empty sets that are necessarily created to obtain the aforementioned alternation
* When previous step is achieved, and we're about to combine commands with the Chronology, we will obviously make sure that the interval starts orders is respected. Concerning the order of the interval ends, we have three options :
    * 1) respect the temporal order of the model
    * 2) respect the order of the ends of intervals of the model
    * 3) respect the association of starts and ends of intervals (of the model AND the commands)

Midifile Performer must implement both steps 3) from previous description (see Bernard's advice)

In the definitions of the combine_n functions, the 4 lines correspond to the 4 cases described in 3.3.2 :

* The first case is when we can find in the model a series of two elements M1_start and M2_end and the commands begin with an interval start c_begin(t)
* The second case is when the commands begin with an end of interval
* The third case concerns punctual events
* The last case corresponds to the end of one of the two chronologies

It looks like in combine_0, the 2nd case is omitted (makes sense because combine_0 doesn't consider ends of intervals, only punctual commands)

### advice from Bernard and Jean :

#### Bernard :

Si tu pars d'une page blanche, le mieux c'est de lire l'article en pièce jointe plutôt que de t'enfoncer dans le code Java. Le point clé est le pré-processing de la partition (3.3.1), à la fin de cette section il y a mention d'une étape "optionelle" mais il faut l'implémenter (pour répondre à "autres problématiques du genre" ;-)). Après, les algorithmes qui prennent une partition et les commandes clavier sont dans 3.3.2. C'est combine_3 qu'il faut implémenter. Les algos sont en style direct (post-mortem, les commandes sont connues à priori), à toi de voir comment tu veux être réactif aux commandes : push/pull. MidifilePerformer est massivement en style pull (en lecture bloquante sur les commandes) parce que la partition était vue aussi comme un flux. Mais si l'ensemble de la partition est connue statiquement, le style push peut être envisager.

#### Jean :

Deux problèmes soulevés :

1. Répétition d’une même note : c'est-à-dire une note liée à elle-même dans la suite de la chronologie.

Il me semble que la note déclenchée primitivement par une touche est effectivement interrompue puis redéclenchée à l’enfoncement d’une autre touche de commande. Mais il faudrait vérifier s’il n’y a pas superposition des deux Notes On pendant l’overlap du jeu des deux touches. Et surtout quel relâchement va couper l’une ou l’autre de la même note. J’ai souvenir que, malheureusement, le premier relâchement coupe la note, la deuxième note ne tient pas et le deuxième relâchement n’a pas d’effet ! Si tu as pratiqué Max/Msp, tu peux trouver dans makenote des arguments Repeatmodes qui contournent cette situation, je crois.

2. Traitement des durées :

Dans la polyphonie et le contrepoint, il y a trois types de notes : 

* Les notes dont la durée remplit exactement le Δ-Time, c'est-à-dire l’intervalle entre deux enfoncements de touches successifs. (IOI, Inter Onset Interval)
* Les notes dont la durée englobe plusieurs Δ-Times, par ex : une ronde tenue pendant quatre noires ou tout autre combinaison de valeurs pour une ronde.
* Les notes dont la durée est inférieure au Δ-Time en cours par ex : des notes détachées dans un jeu staccato.

Dans les trois cas, la note Off se trouve à différents endroits.

* La fin de la note est synchrone avec le nouvel enfoncement : durée = Δ-Time
* La fin de la note est synchrone avec un enfoncement ultérieur : durée > Δ-Time, ou
* La fin de la note est synchrone avec un relâchement ultérieur : durée > Δ-Time
* La fin de la note est avant le nouvel enfoncement : durée < Δ-Time

Tous les questionnements et difficultés qu’on a eus avec Bernard concernent le lien de la note Off à la commande On ou Off.

Dans la formalisation générale on a considéré le dispositif fonctionnant avec une seule touche ; les touches de commandes ne sont donc pas identifiées. Si pendant le jeu quatre touches ont été enfoncées successivement et maintenues enfoncées, quatre Notes On se superposent, le premier relâchement (quelque soit la touche que l’on quitte) arrêtera le premier son déclenché.

C’est pour la facilité du jeu et de l’expression, que plusieurs touches sont nécessaires pour l’articulation legato/staccato, et pour la virtuosité, la rapidité, les trilles etc. 

### CR 29/03/2022 JH BPS GK JL

#### Jean

À propos d'une éventuelle gestion dynamique automatisée de la fonction unmeet :

* dans certains fichiers MIDI il pourrait parfois y avoir sur la piste 1 des métadonnées permettant d'humaniser l'interprétation des partitions
    * voir si on peut utiliser ces messages standards pour inférer de règles de processing des fichiers MIDI
    * trouver QUELS SONT CES MESSAGES !!! (a priori : tempo, vélocité max, rien de vraiment mystique, ni temporel hélas ...)

Projets connexes :

* Midi Humanizer
* Touch pianist
* MuseScore

#### Bernard

Notes en vue de futures modifications dans le but de mieux coller au code java :

* chronology : class de base avec méthode virtuelle get, spécialisées et chaînées
* BlockingLinkedList -> transformer les push en pull
    * --> on met la liste en tête et on y branche les chronologies successives
* peek : une chronologie avec une méthode peek en plus -> on récupère la tête de queue sans la popper
* Tous les évènements non-note sont mergés avec les note offs et envoyés d'un coup

