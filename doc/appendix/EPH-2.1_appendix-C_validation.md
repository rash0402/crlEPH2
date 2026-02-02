---
title: "EPH v2.1: Appendix C — Numerical Validation Plan"
type: Validation_Plan
status: "🟡 In Progress"
version: 2.1.0
date_created: "2026-02-01"
date_modified: "2026-02-02"
author: "Hiroshi Igarashi"
institution: "Tokyo Denki University"
parent_document: "EPH-2.1_main.md"
appendix_id: "C"
tech_stack:
  - "JAX 0.4.x"
  - "Brax 0.10.x"
  - "Matplotlib"
  - "NumPy"
---

# EPH v2.0 Numerical Validation Plan

## 1. Overview

### 1.1 Validation Objectives

| ID | Objective | Theorem | Success Criterion |
|----|-----------|---------|-------------------|
| V1 | β_c dependency on z, τ_haze | Thm 1 | β_c ∝ 1/(z-1) within 10% |
| V2 | Phase transition detection | Thm 1 | Order parameter discontinuity at β_c |
| V3 | 2:6:2 optimality | Thm 2 | F_swarm(2:6:2) < F_swarm(others) |
| V4 | Lyapunov descent | Thm 3 | V[k+1] < V[k] monotonically |
| V5 | Convergence rate ρ | Thm 3 | ρ_empirical ≈ 0.02 ± 0.01 |

### 1.2 Tech Stack

```
JAX 0.4.x          - Automatic differentiation, JIT compilation
Brax 0.10.x        - Physics simulation (optional, for robotics)
Equinox            - Neural network modules
Optax              - Optimizers
Matplotlib         - Visualization
```

---

## 2. Validation V1: β_c Parameter Dependency

### 2.1 Theoretical Prediction

$$\beta_c = \frac{1/\tau_{\text{haze}} - \bar{h}(1-\bar{h})\kappa\eta L_f^2}{2(z-1)}$$

### 2.2 Test Cases

| Case | z | τ_haze | Expected β_c |
|------|---|--------|--------------|
| Base | 6 | 1.0 | 0.098 |
| Z-low | 4 | 1.0 | 0.163 |
| Z-high | 10 | 1.0 | 0.054 |
| τ-low | 6 | 0.5 | 0.195 |
| τ-high | 6 | 2.0 | 0.049 |

### 2.3 Measurement Method

1. Run simulation for each β ∈ [0, 0.3] with step 0.01
2. Measure order parameter: $\phi = \langle |h_i - \bar{h}| \rangle$
3. Find β where $d\phi/d\beta$ is maximum → β_c^empirical
4. Compare with theoretical prediction

---

## 3. Validation V2: Phase Transition Detection

### 3.1 Order Parameter Definition

$$\phi(\beta) = \frac{1}{N} \sum_{i=1}^N |h_i - \bar{h}|$$

- $\phi \approx 0$: Disordered (homogeneous haze)
- $\phi > 0$: Ordered (heterogeneous domains)

### 3.2 Susceptibility

$$\chi(\beta) = N \left( \langle \phi^2 \rangle - \langle \phi \rangle^2 \right)$$

Peak in $\chi$ indicates critical point.

### 3.3 Expected Results

```
φ(β)
│      ╭──────
│     ╱
│    ╱
│───╱
└───────────────── β
   β_c
```

---

## 4. Validation V3: 2:6:2 Optimality

### 4.1 Distribution Comparison

| Distribution | w_L | w_F | w_R | Label |
|--------------|-----|-----|-----|-------|
| 2:6:2 | 0.20 | 0.60 | 0.20 | Optimal |
| 1:8:1 | 0.10 | 0.80 | 0.10 | Leader-sparse |
| 3:4:3 | 0.30 | 0.40 | 0.30 | Balanced |
| 1:1:1 | 0.33 | 0.34 | 0.33 | Uniform |
| 4:2:4 | 0.40 | 0.20 | 0.40 | Follower-sparse |

### 4.2 Performance Metrics

1. **F_swarm**: Total swarm free energy
2. **Task completion time**: Time to reach goal
3. **Collision rate**: Number of collisions / time
4. **Fatigue variance**: Var(Φ_i) across agents

### 4.3 Expected Results

```
F_swarm
│
│ ●4:2:4
│   ●1:1:1
│     ●3:4:3
│       ●1:8:1
│         ●2:6:2  ← minimum
└───────────────────────
```

---

## 5. Validation V4 & V5: Convergence

### 5.1 Lyapunov Function

$$V[k] = \sum_i F_{\text{EPH},i}[k] + \frac{\mu}{2} \sum_i \|h_i[k] - h_i^*\|^2$$

### 5.2 Convergence Test

1. Initialize with random state
2. Run EPH update for K=500 steps
3. Record V[k] at each step
4. Fit: $\log(V[k] - V^*) = -\rho k + c$
5. Extract empirical ρ

### 5.3 Expected Results

```
log(V[k] - V*)
│ ╲
│  ╲
│   ╲  slope = -ρ ≈ -0.02
│    ╲
│     ╲
└──────────────────── k
```

---

## 6. Implementation Structure

```
eph_validation/
├── __init__.py
├── core/
│   ├── eph_agent.py      # Single agent EPH dynamics
│   ├── swarm.py          # Multi-agent coordination
│   ├── spm.py            # Saliency Polar Map
│   └── predictor.py      # World model f_θ
├── validation/
│   ├── v1_beta_c.py      # β_c dependency test
│   ├── v2_phase.py       # Phase transition detection
│   ├── v3_262.py         # 2:6:2 optimality test
│   ├── v4_lyapunov.py    # Convergence test
│   └── runner.py         # Main validation runner
├── utils/
│   ├── metrics.py        # Order parameter, susceptibility
│   └── visualization.py  # Plotting functions
└── configs/
    └── default.yaml      # Default parameters
```

---

## 7. Timeline

| Week | Task | Deliverable |
|------|------|-------------|
| W1 | Core implementation | eph_agent.py, swarm.py |
| W2 | V1, V2 validation | β_c plot, phase diagram |
| W3 | V3 validation | Distribution comparison table |
| W4 | V4, V5 validation | Convergence plot, ρ estimate |

---

## 8. Success Criteria Summary

| Validation | Criterion | Tolerance |
|------------|-----------|-----------|
| V1 | β_c^emp / β_c^theory ∈ [0.9, 1.1] | ±10% |
| V2 | Clear φ discontinuity at β_c | Visual |
| V3 | F_swarm(2:6:2) < F_swarm(others) | Statistical (p<0.05) |
| V4 | V[k+1] ≤ V[k] for all k | Monotonic |
| V5 | ρ_emp ∈ [0.01, 0.03] | ±50% of 0.02 |

---

*Created: 2026-02-01*
