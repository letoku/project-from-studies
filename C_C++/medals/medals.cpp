#include <array>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#define NUM_OF_MEDALS_TYPES 3

using namespace std;

using LineNumber = unsigned long long int;
using Position = unsigned long long int;  // Position in the classification.

using Weight = unsigned int;
using WeightsArray = array<Weight, NUM_OF_MEDALS_TYPES>;

using MedalType =
    unsigned int;  // For indicating what type of medal country received.
using MedalCountNumber =
    unsigned long long int;  // For indicating how many medals each country
                             // received.
using MedalsArray = array<MedalCountNumber, NUM_OF_MEDALS_TYPES>;
using MedalsDataMap = unordered_map<string, MedalsArray>;

using Score = unsigned long long int;  // For score used in classification.
using CountryScore = pair<string, Score>;

static void log_error(LineNumber line_number) {
  cerr << "ERROR " << line_number << endl;
}

static bool compare_countries(const CountryScore& a, const CountryScore& b) {
  if (a.second < b.second) return true;
  if (a.second > b.second) return false;
  if (a.first > b.first) return true;
  return false;
}

static void add_empty_country(string country, MedalsDataMap& medals_data) {
  MedalsArray no_medals_array = {};
  medals_data.insert(make_pair(country, no_medals_array));
}

static void add_medal(string name, MedalType medal,
                      MedalsDataMap& medals_data) {
  if (!medals_data.contains(name) ||
      medal == 0) {  // If country already was added, we don't treat medal 0 as
    // an error(it wasn't specified what to do in this case).
    add_empty_country(name, medals_data);
  }

  if (medal > 0) {
    medals_data[name][medal - 1]++;
  }
}

static void remove_medal(string name, MedalType medal,
                         MedalsDataMap& medals_data, LineNumber line_number) {
  if (!medals_data.contains(name) || medals_data[name][medal - 1] == 0) {
    // If country already was added, we don't treat medal 0 as an
    // error(it wasn't specified what to do in this case).
    log_error(line_number);
  } else {
    medals_data[name][medal - 1]--;
  }
}

static Score calculate_score_for_country(WeightsArray& weights,
                                         MedalsArray& medals) {
  Score score = 0;
  for (MedalType i = 0; i < NUM_OF_MEDALS_TYPES; i++) {
    score += (Score)weights[i] * medals[i];
  }
  return score;
}

static vector<CountryScore> create_classification(WeightsArray& weights,
                                                  MedalsDataMap& medals_data) {
  auto size = medals_data.size();
  vector<CountryScore> classification(size);
  Position position = 0;
  for (auto& it : medals_data) {
    Score score = calculate_score_for_country(weights, it.second);
    classification[position] = make_pair(it.first, score);
    position++;
  }

  sort(classification.begin(), classification.end(), compare_countries);
  return classification;
}

static void print_classification(vector<CountryScore>& classification) {
  Position current_place = 1;
  long long int i = classification.size() - 1;
  Score last_score = -1;
  Position places_shift = 0;
  while (i >= 0) {
    if (classification[i].second != last_score) {
      last_score = classification[i].second;
      current_place += places_shift;
      places_shift = 1;
    } else {
      places_shift++;
    }
    cout << current_place << ". " << classification[i].first << endl;
    i--;
  }
}

int main() {
  string line;
  const regex add_medal_regex_pattern(string(
      ("(^[A-Z].*[A-z]) ([0-" + to_string(NUM_OF_MEDALS_TYPES) + "])$")));
  const regex remove_medal_regex_pattern(
      "^-([A-Z].*[A-z]) ([1-" + to_string(NUM_OF_MEDALS_TYPES) + "])$");
  const regex print_classification_regex_pattern(
      R"(^=([1-9]\d{0,5}) ([1-9]\d{0,5}) ([1-9]\d{0,5})$)");

  MedalsDataMap medals_data;

  LineNumber i = 1;
  while (getline(cin, line)) {
    smatch m;  // Regex matches are stored here. On index 1 there is first ()
               // group.

    if (regex_match(line, m, add_medal_regex_pattern)) {
      string country_name = m[1];
      MedalType medal = static_cast<MedalType>(stoul(m[2]));
      add_medal(country_name, medal, medals_data);

    } else if (regex_search(line, m, remove_medal_regex_pattern)) {
      string country_name = m[1];
      MedalType medal = static_cast<MedalType>(stoul(m[2]));
      remove_medal(country_name, medal, medals_data, i);

    } else if (regex_search(line, m, print_classification_regex_pattern)) {
      WeightsArray weights;
      for (MedalType i = 0; i < NUM_OF_MEDALS_TYPES; i++) {
        weights[i] = static_cast<Weight>(stoul(m[i + 1]));
      }
      vector<CountryScore> classification =
          create_classification(weights, medals_data);
      print_classification(classification);

    } else {
      log_error(i);
    }
    i++;
  }

  return 0;
}