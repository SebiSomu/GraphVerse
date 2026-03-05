import os
import urllib.request
import time

FLAG_MAPPING = {
    "Spanish": "es", "French": "fr", "Portuguese": "pt", "Italian": "it", "Romanian": "ro",
    "Catalan": "es", "Galician": "es-ga", "Occitan": "fr", "English": "gb", "German": "de",
    "Dutch": "nl", "Swedish": "se", "Danish": "dk", "Norwegian": "no", "Afrikaans": "za",
    "Russian": "ru", "Ukrainian": "ua", "Polish": "pl", "Czech": "cz", "Bulgarian": "bg",
    "Serbian": "rs", "Belarusian": "by", "Slovak": "sk", "Croatian": "hr", "Slovenian": "si",
    "Mandarin": "cn", "Cantonese": "hk", "Wu": "cn", "Burmese": "mm", "Tibetan": "cn",
    "Hindi": "in", "Urdu": "pk", "Bengali": "bd", "Punjabi": "in", "Marathi": "in",
    "Persian": "ir", "Kurdish": "iq", "Pashto": "af", "Finnish": "fi", "Estonian": "ee",
    "Hungarian": "hu", "Arabic": "sa", "Hebrew": "il", "Amharic": "et", "Tigrinya": "er",
    "Maltese": "mt", "Japanese": "jp", "Korean": "kr", "Tamil": "in", "Telugu": "in",
    "Kannada": "in", "Malayalam": "in", "Swahili": "tz", "Hausa": "ng", "Yoruba": "ng",
    "Zulu": "za", "Igbo": "ng", "Fula": "sn", "Shona": "zw", "Indonesian": "id",
    "Malay": "my", "Tagalog": "ph", "Javanese": "id", "Vietnamese": "vn", "Thai": "th",
    "Turkish": "tr", "Azerbaijani": "az", "Uzbek": "uz", "Kazakh": "kz", "Greek": "gr",
    "Armenian": "am", "Georgian": "ge", "Albanian": "al", "Basque": "es-pv"
}

os.makedirs("flags", exist_ok=True)

user_agent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'

success_count = 0
for lang, code in FLAG_MAPPING.items():
    try:
        url = f"https://flagcdn.com/h40/{code}.png"
        path = f"flags/{lang}.png"
        if not os.path.exists(path):
            req = urllib.request.Request(url, headers={'User-Agent': user_agent})
            with urllib.request.urlopen(req) as response, open(path, 'wb') as out_file:
                out_file.write(response.read())
            time.sleep(0.05) # Be gentle to the API
        success_count += 1
    except Exception as e:
        print(f"Failed {lang}: {e}")
        # fallback to a generic placeholder if it really fails
        if code in ["es-ga", "es-pv", "ad"]:
            try:
                url = "https://flagcdn.com/h40/es.png"
                req = urllib.request.Request(url, headers={'User-Agent': user_agent})
                with urllib.request.urlopen(req) as response, open(path, 'wb') as out_file:
                    out_file.write(response.read())
            except:
                pass

print(f"Finished downloading {success_count} flags.")
