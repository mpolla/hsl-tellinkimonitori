# Tellinkimonitori

Näytä HSL:n kaupunkipyöräasemien tilanne helposti liikuteltavalla e-paperinäytöllä.

<img alt="Tellinkimonitori jääkaapin ovessa" src="https://github.com/mpolla/hsl-tellinkimonitori/blob/master/img/tellinkimonitori-jaakaappi.jpeg?raw=true" width="600" />

## Lista materiaaleista

* [WaveShare 4.2" e-paperinäyttö](https://www.waveshare.com/wiki/4.2inch_e-Paper_Module)
* [LOLIN Wemos D1 mini Pro](https://www.aliexpress.com/item/32724692514.html) kehitysalusta tai mikä tahansa muu ESP8266 perheen alusta
* 4 kpl kestomagneetteja (lieriön halkaisija 8 mm)
* 3.7 V LiPo akku (esim. 380 mAh)
* Kytkentälankaa, juotostinaa
* PLA-tulostusmateriaalia 3D-printteriä varten
* Työkalut: kolvi, sivuleikkurit, 3D-tulostin

## Ohjeet

### Kehitysalustan ohjelmointi

- Asenna Arduino IDE kehitystyökalu
- Kohdassa *Sketch → Library manager* asenna tarvittavat kirjastot
  *Time* (aikatiedon käsittely), *GxEPD* (e-paperinäytön tuki) ja
  *Adafruit GFX* (fontit)
- Avaa oheinen koodilistaus `hsl-tellinkimonitori.ino`
- Kytke kehitysalusta USB-väylään ja tarkista että oikea portti on
  valittuna Arduino IDE:ssä kohdassa *Tools → Port*
- Tee tarvittavat muokkaukset ohjelmakoodiin
  - Aseta oman WLAN-verkkosi nimi ja salasana kohdissa
    `WLAN-VERKKONI-NIMI` ja `WLAN-VERKKONI-SALASANA`
  - Valitse minkä kaupunkipyröasemien tiedot (tilaa on neljälle)
    haluat näyttää. Tarkista asemien tunnistenumerot [HSL:n sivuilta](https://kaupunkipyorat.hsl.fi/fi/helsinki/stations)
    Esim. Senaatintori on `014`
  - Jos haluat näyttää lämpötilan, rekisteröidy [OpenWeather](https://openweathermap.org)
    palveluun ja aseta oma koodisi ohjelmakoodissa kohtaan
    `MINUN-APPID-KOODINI`
- Ohjelmoi kehitysalusta valitsemalla *Upload*

### Elektroniikkakytkennät

- Yhdistä kehitysalustan ja e-paperinäytön kytkennät juottamalla kiinni SPI-väylän liittimet seuraavasti:
  - `VCC` - `3V3` (punainen)
  - `GND` - `GND` (musta)
  - `DIN` - `GPIO13` (eli D7, kuvassa sininen johdin)
  - `CLK` - `GPIO14` (eli D5, kuvassa keltainen johdin)
  - `CS` - `GPIO5` (eli D1, kuvassa oranssi johdin)
  - `DC` - `GPIO0` (eli D3, kuvassa vihreä johdin)
  - `RST` - `GPIO2` (eli D4, kuvassa valkoinen johdin)
  - `BUSY` - `GPIO4` (eli D2, kuvassa lila johdin)
- Kytke LiPo-akku kiinni kehitysalustaan (punainen johdin + napaan)

[Kuva](https://github.com/mpolla/hsl-tellinkimonitori/blob/master/img/d1-kytkennat.jpeg?raw=true)
e-paperinäytön ja kehitysalustan kytkennöistä.

### Deep sleep -toiminnon aktivoiminen

- Varmista vielä, että kehitysalusta on ohjelmoitu ja kytkennät
  toimivat. Tee tarvittaessa muutoksia, ohjelmoi kehitysalusta
  uudelleen.
- Kun kaikki on valmista, juota yhteen kehitysalustan portit `GPIO16`
  (eli `D0`) ja `RST` (kuvassa lyhyet harmaat johtimet). Vinkki: Kytke
  kumpaankin porttiin oma kytkentälanka, jonka voit tarvittaessa
  yhdistää ja irroittaa tarpeen mukaan. Kehitysalustan ohjelmoiminen
  vaatii, että kytkentä on irti.

### Kotelo

- Asenna slicer-ohjelma (esim. Cura tai PrusaSlicer) jolla oheisten
  STL-tiedostojen kuvaus kotelon muodosta muunnetaan 3D-tulostimen
  ymmärtämiksi ohjekisi gcode-tiedostoksi.
- Varmista, että slicer-ohjelmassa on valittu käyttämällesi
  3D-tulostimelle oikeat asetukset.
- Slicer-ohjelmasta riippuen valitse esim. *Import STL file* ja
  tarvittaessa säädä tulostasetuksia. Tukirakenteille (support) ei ole
  tässä tapauksessa tarvetta.
- Siirrä gcode-tiedosto muistitikulle tai SD-kortille valitsemalla
  esim. *Export G-code*.
- Syötä luomasi gcode-tiedostot 3D-tulostimelle ja käynnistä tulostus.
- Kokoa osat: sovita paikoilleen e-paperinäyttö, kehitysalusta ja
  akku. Käsittele johtimia varovaisesti, ettet katkaise yhteyksiä.
- Napsauta takakannen magneetit kiinni. Kiinnitä takakansi paikoilleen
  niin, että magneetit ovat kotelon sisällä.

## Taustaa

Mikrobitti 2/2018 [Rajapinnasta e-paperille](https://www.mikrobitti.fi/neuvot/nain-rakennat-ja-koodaat-oman-mininayton-mikrokontrollerista-ja-e-paperista-soveltuu-bussiaikatauluihin-porssiin-tai-vaikka-pelistatistiikkaan/db4d396c-53be-4205-b6f8-0ecdc055f860)
