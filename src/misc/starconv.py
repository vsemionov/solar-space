import sys


header = """\
//Stars file, generated by starconv
// using the Yale Bright Star Catalog
//
//NOTE: THE STARS MUST BE SORTED BY MAGNITUDE!!!
//
//Format:
//number of stars
//star line 1
//star line 2
//...........
//
//Star line format: (separator=='|')
//Dec, degrees [-90:90]
//RA, degrees [0:360)
//SEPARATOR
//star magnitute, mag
//SEPARATOR
//B-V color index, mag

"""

fields = {
            "DE-": (84,),
            "DEd": (85, 86),
            "DEm": (87, 88),
            "DEs": (89, 90),
            "RAh": (76, 77),
            "RAm": (78, 79),
            "RAs": (80, 83),
            "Vmag": (103, 107),
            "B-V": (110, 114)
            }


stars = []


class Star:
    def __init__(self):
        self.dec = 0.0
        self.ra = 0.0
        self.mag = 0.0
        self.bv = 0.0


def usage():
    print("Usage: starconv.py catalog starmap", file=sys.stderr)

def parse_field(line, field):
    byte_range = fields[field]
    if len(byte_range) == 2:
        index1 = byte_range[0] - 1
        index2 = byte_range[1]
        return float(line[index1:index2])
    else:
        index = byte_range[0] - 1
        return float(line[index] + "1")

def valid_line(line):
    return line[fields["Vmag"][0]-1+2] == '.'

def parse(line):
    if not valid_line(line):
        return None
    star = Star()
    star.dec = parse_field(line, "DE-") * (parse_field(line, "DEd") + parse_field(line, "DEm")/60 + parse_field(line, "DEs")/3600)
    star.ra = 15 * (parse_field(line, "RAh") + parse_field(line, "RAm")/60 + parse_field(line, "RAs")/3600)
    star.mag = parse_field(line, "Vmag")
    try:
        star.bv = parse_field(line, "B-V")
    except ValueError:
        star.bv = 0.0 # light blue if B-V is not specified
    return star


if len(sys.argv) != 3:
    usage()
    sys.exit(1)

print("reading and parsing input data...")
catalog = open(sys.argv[1], "rt")
for line in catalog:
    line = line.strip('\n')
    star = parse(line)
    if star is not None:
        stars.append(star)
catalog.close()

print("sorting...")
stars.sort(key=lambda star: star.mag, reverse=True)

print("writing output data...")
starmap = open(sys.argv[2], "wt")
starmap.write(header)
starmap.write(str(len(stars)) + "\n")
for star in stars:
    line = "{} {} | {} | {}\n".format(star.dec, star.ra, star.mag, star.bv)
    starmap.write(line)
starmap.close()

print("done.")
