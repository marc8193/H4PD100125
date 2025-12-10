# Schema Registry
1.1. Hvordan virker Schema registriet? Det beskriver strukturen på det data, som producers sender til Kafka.
   		   		  		 			 Når en producer sender en besked, gemmer den ikke selve schemaet i beskeden – kun et lille schema-ID.

1.2 Hvorfor er det smart? Fordi det sikrer, at producer og consumer altid tolker data på samme måde, da begge bruger den samme versionsstyrede datastruktur.

2. Hvordan benytter mit producer/consumer software Schemas?
   - Produceren validerer sine data mod schemaet i Schema Registry. Når den sender en besked, tilføjer den kun schema-ID’et til payloaden.
   - Consumeren læser schema-ID’et fra beskeden, henter det tilsvarende schema fra Schema Registry og bruger det til at deserialisere og tolke dataen korrekt.


3. Hvad betyder det for performance/throughput? Da der kun sendes et schema-ID i hver besked, og ikke hele schemaet bliver beskederne mindre i størrelse, hvilket giver mindre netværkstrafik, hurtigere overførsel og dermed bedre throughput.

4. Hvad kan jeg gøre, hvis jeg får brug for at fjerne/tilføje/ændre en del af en compleks datatype, som allerede findes i store mængder i et topic, og som jeg har consumere der bruger flittigt af? Fordi der benyttes en binær protokol, er det nemt at tilføje nye felter uden at ødelægge eksisterende implementeringer.

| Før       | Efter     | Type   |
|-----------|-----------|--------|
| id        | id        | int    |
| navn      | navn      | string |
| alder     | alder     | int    |
|           | adresse   | string |
|           | region    | string |