

hebrewLetters = {'א' : "AL", 'ב' : "BE", 'ג' : "GI", 'ד' : "DA", 'ה' : "HE", 'ו' : "VA", 'ז' : "ZA", 'ח' : "CH",
                 'ט' : "TE", 'י' : "YO", 'ך' : "FKA", 'כ' : "KA", 'ל' : "LA", 'ם' : "FME", 'מ' : "ME", 'ן' : "FNU",
                 'נ' : "NU", 'ס' : "SA", 'ע' : "AI", 'ף' : "FPE", 'פ' : "PE", 'ץ' : "FTS", 'צ' : "TS", 'ק' : "KU",
                 'ר' : "RE", 'ש' : "SH", 'ת' : "TA", ' ' : "SP"}


def readNamesFile(fromFilePath: str, toFilePath : str):
    namesFile = open(fromFilePath, mode="r", encoding="utf-8")
    cArrayFile = open(toFilePath, mode="w")

    allNames = namesFile.readlines()

    unsupportedChars = {}

    longestName = 0
    namesCount = len(allNames)

    cArrayFile.write("{\n")
    for nameIndex in range(len(allNames)):
        name = allNames[nameIndex]
        if longestName < len(name):
            longestName = len(name)
        cArrayFile.write("{")
        for charIndex in range(len(name) - 1):
            if name[charIndex] in hebrewLetters:
                cArrayFile.write(hebrewLetters[name[charIndex]])
                if charIndex < len(name) - 2:
                    cArrayFile.write(",")
            elif name[charIndex] not in unsupportedChars:
                unsupportedChars[name[charIndex]] = 1
            else:
                unsupportedChars[name[charIndex]] += 1
        cArrayFile.write("}")
        if nameIndex < len(allNames) - 1:
            cArrayFile.write(",")
        if nameIndex % 5 == 4:
            cArrayFile.write("\n")
    cArrayFile.write("\n}")

    cArrayFile.write(f"\nLongest Name: {longestName}")
    cArrayFile.write(f"\nNames Count: {namesCount}")
    cArrayFile.write(f"\nUnsupported Chars: {unsupportedChars}")

    namesFile.close()
    cArrayFile.close()


def generateNNames(filePath : str, count : int):
    file = open(filePath, mode="w", encoding="utf-8")
    name = "name"
    for i in range(count):
        pass



def main():
    readNamesFile("names", "cArray")


main()
