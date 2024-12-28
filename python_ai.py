import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from dataclasses import dataclass
from typing import Dict, Tuple, List
from IPython.display import display

@dataclass
@dataclass
class FuzzySet:
    name: str
    ranges: tuple  # (start, end) for linear, (start,peak,end) for triangle, (start,left_peak,right_peak,end) for trapezoid
    type: str  # 'linear_down', 'linear_up', 'triangle', 'trapezoid'

class FuzzyVariable:
    def __init__(self, name: str, sets: List[FuzzySet], x_range: tuple):
        self.name = name
        self.sets = sets
        self.x_range = x_range

    def calculate_membership(self, value: float) -> Dict[str, float]:
        result = {}
        for fuzzy_set in self.sets:
            membership = 0.0

            if value < min(fuzzy_set.ranges) or value > max(fuzzy_set.ranges):
                membership = 0.0
            elif fuzzy_set.type == 'linear_down':
                start, end = fuzzy_set.ranges
                if value <= start:
                    membership = 1.0
                elif value < end:
                    membership = (end - value) / (end - start)
                else:
                    membership = 0.0
            elif fuzzy_set.type == 'linear_up':
                start, end = fuzzy_set.ranges
                if value <= start:
                    membership = 0.0
                elif value < end:
                    membership = (value - start) / (end - start)
                else:
                    membership = 1.0
            elif fuzzy_set.type == 'triangle':
                start, peak, end = fuzzy_set.ranges
                if start < value <= peak:
                    membership = (value - start) / (peak - start)
                elif peak < value < end:
                    membership = (end - value) / (end - peak)
                elif value == peak:
                    membership = 1.0
            elif fuzzy_set.type == 'trapezoid':
                start, left_peak, right_peak, end = fuzzy_set.ranges
                if start < value <= left_peak:
                    membership = (value - start) /(max(left_peak - start, 1e-10))
                elif left_peak <= value <= right_peak:
                    membership = 1.0
                elif right_peak < value < end:
                    membership = (end - value) / (end - right_peak)

            if membership > 0:
                result[fuzzy_set.name] = membership

        return result

class PlantWateringFuzzySystem:
    def __init__(self):
        # Define fuzzy variables with additional trapezoidal examples
        self.temperature = FuzzyVariable("Temperature", [
            FuzzySet("Dingin", (0, 0, 15, 20), "trapezoid"),
            FuzzySet("Normal", (15, 20, 25, 30), "trapezoid"),
            FuzzySet("Panas", (25, 30, 50, 50), "trapezoid")
        ], (0, 50))

        self.humidity = FuzzyVariable("Humidity", [
            FuzzySet("Kering", (0, 0, 20, 30), "trapezoid"),
            FuzzySet("Lembab", (20, 35, 70, 80), "trapezoid"),
            FuzzySet("Basah", (70, 80, 100, 100), "trapezoid")
        ], (0, 100))

        self.light = FuzzyVariable("Light", [
            FuzzySet("Terang", (0, 0, 400, 800), "trapezoid"),
            FuzzySet("Redup", (400, 800, 1400, 2000), "trapezoid"),
            FuzzySet("Gelap", (1400, 2000, 3200, 3200), "trapezoid")
        ], (0, 3400))

        # Define watering output variable for Mamdani with trapezoidal membership
        self.watering_output = FuzzyVariable("Watering", [
            FuzzySet("Tidak Perlu Air", (0, 0, 20, 25), "trapezoid"),
            FuzzySet("Perlu Air Sedikit", (20, 25, 45, 50), "trapezoid"),
            FuzzySet("Perlu Air", (45, 50, 70, 75), "trapezoid"),
            FuzzySet("Perlu Air Banyak", (70, 80, 100, 100), "trapezoid")
        ], (0, 100))

        # Sugeno output values remain the same as they are singleton values
        self.sugeno_outputs = {
            "Tidak Perlu Air": 0,
            "Perlu Air Sedikit": 25,
            "Perlu Air": 50,
            "Perlu Air Banyak": 75
        }

        self.rules = self._initialize_rules()

    def _initialize_rules(self) -> Dict:
        """Initialize the complete rule base for the fuzzy system."""
        return {
            'Dingin \ Gelap': {
                'Kering': 'Tidak Perlu Air',
                'Lembab': 'Tidak Perlu Air',
                'Basah': 'Tidak Perlu Air'
            },
            'Normal \ Gelap': {
                'Kering': 'Tidak Perlu Air',
                'Lembab': 'Tidak Perlu Air',
                'Basah': 'Tidak Perlu Air'
            },
            'Panas \ Gelap': {
                'Kering': 'Perlu Air',
                'Lembab': 'Perlu Air Sedikit',
                'Basah': 'Perlu Air Sedikit'
            },
            'Dingin \ Redup': { # optimal = redup
                'Kering': 'Perlu Air',
                'Lembab': 'Tidak Perlu Air',
                'Basah': 'Tidak Perlu Air'
            },
            'Normal \ Redup': {
                'Kering': 'Perlu Air',
                'Lembab': 'Tidak Perlu Air',
                'Basah': 'Tidak Perlu Air'
            },
            'Panas \ Redup': {
                'Kering': 'Perlu Air',
                'Lembab': 'Air Sedikit',
                'Basah': 'Air Sedikit'
            },
            'Dingin \ Terang': {
                'Kering': 'Perlu Air',
                'Lembab': 'Perlu Air Sedikit',
                'Basah': 'Tidak Perlu Air'
            },
            'Normal \ Terang': {
                'Kering': 'Perlu Air',
                'Lembab': 'Perlu Air Sedikit',
                'Basah': 'Tidak Perlu Air'
            },
            'Panas \ Terang': {
                'Kering': 'Perlu Air Banyak',
                'Lembab': 'Perlu Air',
                'Basah': 'Perlu Air'
            }
        }

    def fuzzify(self, temp: float, humid: float, light: float) -> Tuple[Dict, Dict, Dict]:
        """Fuzzify crisp input values."""
        temp_membership = self.temperature.calculate_membership(temp)
        humid_membership = self.humidity.calculate_membership(humid)
        light_membership = self.light.calculate_membership(light)
        return temp_membership, humid_membership, light_membership

    def infer(self, temp_fuzzy: Dict, humid_fuzzy: Dict, light_fuzzy: Dict) -> Tuple[Dict, Dict]:
        """Perform fuzzy inference and return both MIN implications and MAX results."""
        implications = {}  # Store all MIN implications
        max_results = {}   # Store MAX results for each decision

        for temp_cat, temp_val in temp_fuzzy.items():
            for humid_cat, humid_val in humid_fuzzy.items():
                for light_cat, light_val in light_fuzzy.items():
                    key = f"{temp_cat} \ {light_cat}"
                    if key in self.rules:
                        decision = self.rules[key][humid_cat]
                        min_val = min(temp_val, humid_val, light_val)

                        # Create a rule description
                        rule_desc = f"IF Temperature is {temp_cat} AND Humidity is {humid_cat} AND Light is {light_cat} THEN {decision}"

                        # Store the implication
                        if decision not in implications:
                            implications[decision] = []
                        implications[decision].append((rule_desc, min_val))

                        # Update max results
                        if decision not in max_results:
                            max_results[decision] = min_val
                        else:
                            max_results[decision] = max(max_results[decision], min_val)

        return implications, max_results

    def display_rules(self):
        """Display the rule base in a formatted table."""
        df_rules = pd.DataFrame.from_dict(self.rules, orient='index').transpose()
        display(df_rules)

    def print_inference_results(self, implications: Dict, max_results: Dict):
        """Print the inference results in a formatted way."""
        print("\n" + "="*50)
        print("Rules Used and MIN Implications".center(50))
        print("="*50)

        for decision, impl_list in implications.items():
            for rule_desc, min_val in impl_list:
                print(f"{rule_desc} = μ{decision}({min_val:.2f})")

        print("\n" + "="*35)
        print("MAX Implications".center(35))
        print("="*35)

        for decision, max_val in max_results.items():
            print(f"μ{decision} = {max_val:.2f}")

    def plot_membership_functions(self, variable: FuzzyVariable):
        """Plot membership functions for a given fuzzy variable."""
        x = np.linspace(variable.x_range[0], variable.x_range[1], 1000)
        plt.figure(figsize=(9, 4))

        for fuzzy_set in variable.sets:
            y = [variable.calculate_membership(val).get(fuzzy_set.name, 0) for val in x]
            plt.plot(x, y, label=fuzzy_set.name)

        plt.title(f"{variable.name} Membership Functions")
        plt.xlabel("Value")
        plt.ylabel("Membership Degree")
        plt.legend()
        plt.grid(True)
        plt.show()

    def calculate_output_membership(self, x: float, output_set: FuzzySet) -> float:
            """Calculate membership value for output fuzzy sets."""
            if output_set.type == "trapezoid":
                start, left_peak, right_peak, end = output_set.ranges
                if x < start or x > end:
                    return 0
                elif start <= x <= left_peak:
                    return (x - start) /(max(left_peak - start, 1e-10))
                elif left_peak <= x <= right_peak:
                    return 1
                else:
                    return (end - x) / (end - right_peak)
            return 0

    def defuzzify_mamdani(self, max_results: Dict[str, float], num_points: int = 100) -> float:
        """
        Perform Mamdani defuzzification using center of gravity method.
        """
        x_range = np.linspace(0, 100, num_points)
        numerator = 0
        denominator = 0

        for x in x_range:
            # Find maximum membership value at this point across all output sets
            max_membership = 0
            for output_label, rule_strength in max_results.items():
                output_set = next(s for s in self.watering_output.sets if s.name == output_label)
                membership = min(rule_strength, self.calculate_output_membership(x, output_set))
                max_membership = max(max_membership, membership)

            numerator += x * max_membership
            denominator += max_membership

        return numerator / denominator if denominator != 0 else 0

    def defuzzify_sugeno(self, max_results: Dict[str, float]) -> float:
        """
        Perform Sugeno defuzzification using weighted average.
        """
        if not max_results:
          return 0.0
          
        numerator = 0.0
        denominator = 0.0
        
        for label, strength in max_results.items():
            if label in self.sugeno_outputs:
                numerator += strength * self.sugeno_outputs[label]
                denominator += strength
      
        return numerator / denominator if denominator > 0 else 0.0
          

    def plot_mamdani_output(self, max_results: Dict[str, float], crisp_output: float = None):
        """
        Plot the Mamdani output membership functions and aggregated result.
        """
        x = np.linspace(0, 100, 1000)
        plt.figure(figsize=(12, 6))

        # Plot individual output membership functions
        for output_set in self.watering_output.sets:
            y = [self.calculate_output_membership(val, output_set) for val in x]
            plt.plot(x, y, '--', label=f'{output_set.name} (Base)')

        # Plot clipped and aggregated output
        y_aggregated = []
        for xi in x:
            max_membership = 0
            for output_label, rule_strength in max_results.items():
                output_set = next(s for s in self.watering_output.sets if s.name == output_label)
                membership = min(rule_strength, self.calculate_output_membership(xi, output_set))
                max_membership = max(max_membership, membership)
            y_aggregated.append(max_membership)

        plt.plot(x, y_aggregated, 'r-', linewidth=2, label='Aggregated Output')

        if crisp_output is not None:
            plt.axvline(x=crisp_output, color='g', linestyle='-',
                       label=f'Crisp Output ({crisp_output:.2f})')

        plt.title('Mamdani Output Membership Functions and Aggregated Result')
        plt.xlabel('Watering Amount (%)')
        plt.ylabel('Membership Degree')
        plt.legend()
        plt.grid(True)
        plt.show()

    def process_input(self, temp: float, humid: float, light: float) -> Dict[str, float]:
        """
        Process input values and return both Mamdani and Sugeno defuzzified outputs.
        """
        # Fuzzify inputs
        temp_fuzzy, humid_fuzzy, light_fuzzy = self.fuzzify(temp, humid, light)

        # Perform inference
        implications, max_results = self.infer(temp_fuzzy, humid_fuzzy, light_fuzzy)

        # Perform defuzzification
        mamdani_output = self.defuzzify_mamdani(max_results)
        sugeno_output = self.defuzzify_sugeno(max_results)

        # Plot results
        self.plot_mamdani_output(max_results, mamdani_output)

        return {
            'fuzzification': {
                'temperature': temp_fuzzy,
                'humidity': humid_fuzzy,
                'light': light_fuzzy
            },
            'implications': implications,
            'max_results': max_results,
            'mamdani_output': mamdani_output,
            'sugeno_output': sugeno_output
        }

def run_fuzzy_system():
    """Run the fuzzy system with user input."""
    system = PlantWateringFuzzySystem()

    print("Enter the input values:")
    temp = float(input("Temperature (0-50°C): "))
    humid = float(input("Humidity (0-100%): "))
    light = float(input("Light intensity (0-3200 lux): "))

    # Process inputs and get results
    results = system.process_input(temp, humid, light)

    # Display results
    print("\nFuzzification Results:")
    for var_name, memberships in results['fuzzification'].items():
        print(f"{var_name}:", {k: f"{v:.2f}" for k, v in memberships.items()})

    print("\nDefuzzification Results:")
    print(f"Mamdani output (Center of Gravity): {results['mamdani_output']:.2f}%")
    print(f"Sugeno output (Weighted Average): {results['sugeno_output']:.2f}%")

    # Show rule base
    print("\nRule Base:")
    system.display_rules()

    # Display inference results
    system.print_inference_results(results['implications'], results['max_results'])

    # Plot membership functions
    system.plot_membership_functions(system.temperature)
    system.plot_membership_functions(system.humidity)
    system.plot_membership_functions(system.light)

    system.defuzzify_mamdani(results['max_results'])
    system.defuzzify_sugeno(results['max_results'])
    system.plot_mamdani_output(results['max_results'])

if __name__ == "__main__":
    run_fuzzy_system()
