//hex Table for floating digits
/*
* 
* 0. = 0x4C3F
* 1. = 0x4006
* 2. = 0x40DB
* 3. = 0x40CF
* 4. = 0x40E6
* 5. = 0x40ED
* 6. = 0x40FD
* 7. = 0x4007
* 8. = 0x40FF
* 9. = 0x40EF
*/

//Funktion zum anzeigen der Geschwindigkeit
void display_speed(float display_number, char digit0, char digit1) {
	// Lokale Variablen
	int digit2 = 0;
	int digit3 = 0;
	char displaybuffer[4];
	char charbuffer[2];
	String str;

	alpha4.writeDigitAscii(0, digit0);
	alpha4.writeDigitAscii(1, digit1);

	if (display_number < 10) {                        //für Geschwindigkeiten unter 10 kmh soll die Geschwindigkeit mit einem Komma dargestellt werden
		digit2 = display_number;                       //int rundet immer ab, daher kann ich die Kommazahl einfach der Ganzzahl zuweisen, so wird auch 4.9 zu 4
		digit3 = (display_number - digit2) * 10;         //nun wird von kmh die erste Ziffer abgezogen (4.9-4 = 0.9) und wird mit zehn multipliziert, damit wir auch die zweite Ziffer als ganzzahl haben
														 //digit2                              
														 //Hier wird es tricky, da die Library nicht wirklich eine Funktion zum darstellen von Kommazahlen hat, muss ich ein wenig drum herum werkeln
		switch (digit2) {                       //Es wird abgefragt, welchen Wert digit2 hat und es wird dann dementsprechend die Zahl mit Kommapunkt dargestellt

		case 0:
			alpha4.writeDigitRaw(2, 0x4C3F);
			break;
		case 1:
			alpha4.writeDigitRaw(2, 0x4006);
			break;
		case 2:
			alpha4.writeDigitRaw(2, 0x40DB);
			break;
		case 3:
			alpha4.writeDigitRaw(2, 0x40CF);
			break;
		case 4:
			alpha4.writeDigitRaw(2, 0x40E6);
			break;
		case 5:
			alpha4.writeDigitRaw(2, 0x40ED);
			break;
		case 6:
			alpha4.writeDigitRaw(2, 0x40FD);
			break;
		case 7:
			alpha4.writeDigitRaw(2, 0x4007);
			break;
		case 8:
			alpha4.writeDigitRaw(2, 0x40FF);
			break;
		case 9:
			alpha4.writeDigitRaw(2, 0x40EF);
			break;
		}
		//digit3 in char umwandeln. Ich weiß gar nicht, was ich da genau mache, da ich einfach eine anleitung übernomen habe, aber es scheint zu funktionieren
		//wenn man es nicht macht, dann leuchtet nur das entsprechende segment auf dem display und keine zahl.
		//str = String(digit3);
		//str.toCharArray(charbuffer, 5);
		//displaybuffer[3] = charbuffer[0];
		displaybuffer[3] = char(digit3);

		//Schreibe die Zeichen, welche der Funktion mitgegeben wurden und die Vierte Stelle des Displays...

		alpha4.writeDigitAscii(3, displaybuffer[3]);
		//...und gebe es aus!
		alpha4.writeDisplay();
	}

	else if (display_number >= 10) {              //Anzeigen wenn man nun zehn oder schneller fährt
		digit2 = display_number / 10;                //hier wird die linke Ziffer der zweistelligen Zahl ermittelt: 15/10=1.5 -> 1
		digit3 = display_number - (digit2 * 10);       //hier wird die rechte Ziffer der zweistelligen Zahl ermittelt: 15-(1*10) = 5

		//str = String(digit2);             //hier wieder die dubiose umwandlung von int zu char
		//str.toCharArray(charbuffer, 5);
		//displaybuffer[2] = charbuffer[0];
		displaybuffer[2] = char(digit2);

		//str = String(digit3);
		//str.toCharArray(charbuffer, 5);
		//displaybuffer[3] = charbuffer[0];
		displaybuffer[3] = char(digit3);

		//und hier dann wieder das setzen der Buchstaben für jede stelle das displays


		alpha4.writeDigitAscii(2, displaybuffer[2]);
		alpha4.writeDigitAscii(3, displaybuffer[3]);
		//und hier nun der befehl zum anzeigen
		alpha4.writeDisplay();
	}
}

//Interrupt Service Routine
void rpm_ISR() {
	rpm_count++;
}


//Funktion zum berechnen der geschwindigkeit

float calculate_m_per_s(int rpm_count, float diameter)
{
	int time_difference;
	static unsigned long timeold = 0;
	long rpm = 0;
	long rps = 0;
	time_difference = millis() - timeold;       //zeit, wann das letzte mal die funktion ausgeführt wurde (wenn eine bestimmte anzahl an mindestumdrehungen festgestellt wurde)
	timeold = millis();
	rpm = 60000 / time_difference*rpm_count;
	rps = (rpm / 60);
	return (diameter * 3.1415 * rps);
}

float calculate_kmh(float m_per_s)
{
	return (m_per_s * 3.6);
}

float calculate_distance(float distance, float diameter, int rpm_count)
{
	return (distance + (rpm_count * diameter * 3.1415));
}

float calculate_max_kmh(float max_kmh, float kmh)
{
	if (kmh>max_kmh)
	{

		return kmh;
	}
	else
	{
		return max_kmh;
	}
}

//Funktionen zum Glätten der Analogsensoren und zur Kalibrierung der Sensoren

//Von den Analogsensoren werden X werte eingelesen und ein durchschnitt dieser werte erstellt
//Es werden der Pin des Sensors, ein array und der index des array als parameter übergeben und ein
//geglätteter wert zurückgegeben

int smoothing(int pin, int readings_input[], int *index, int max_werte)
{
	int total, average;
	total = 0;
	readings_input[*index] = analogRead(pin);
	*index += 1;
	if (*index >= max_werte) {
		*index = 0;
	}
	for (int i = 0; i < max_werte; i++) {
		total = total + readings_input[i];
	}
	average = total / max_werte;
	return average;
}

