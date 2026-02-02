---
title: "EPH v2.1: Appendix D — Related Work Survey"
type: Literature_Review
status: "✅ Complete"
version: 2.1.0
date_created: "2026-02-01"
date_modified: "2026-02-02"
author: "Hiroshi Igarashi"
institution: "Tokyo Denki University"
parent_document: "EPH-2.1_main.md"
appendix_id: "D"
survey_method: "WebSearch + WebFetch URL verification"
---

# Related Work Survey Report

**Survey Date**: 2026-02-01
**Target**: EPH v2.0: Active Inference for Multi-Agent Coordination with Intrinsic Perceptual Haze and 2:6:2 Role Distribution

---

## Research Overview

**Research Theme**: Implementation of self-organization through phase transition phenomena by introducing Intrinsic Perceptual Haze and 2:6:2 role distribution in swarm agent control using Active Inference based on Free Energy Principle

**Keywords**: Free Energy Principle, Active Inference, Multi-Agent Systems, Perceptual Haze, Phase Transition, 2:6:2 Distribution, Role Heterogeneity

**Novelty Claims**:
1. Theoretical redefinition of "haze" as an adaptive mechanism rather than a defect
2. Derivation of 2:6:2 role distribution from variational principles
3. Analytical determination of phase transition critical point β_c ≈ 0.098

---

## Top 8 Related Papers

### Paper 1: ★★★ (Direct Foundation)

**Authors**: Conor Heins, Beren Millidge, Lancelot Da Costa, Richard P. Mann, Karl J. Friston, Iain D. Couzin
**Title**: Collective behavior from surprise minimization
**Venue**: PNAS, Vol. 121, No. 17, e2320239121 (2024)
**DOI**: 10.1073/pnas.2320239121
**URL**: https://www.pnas.org/doi/10.1073/pnas.2320239121

**Summary**: Demonstrates that collective behavior patterns (cohesion, milling, directed motion) emerge naturally from Active Inference (surprise minimization) without explicit behavioral rules.

**Relevance to EPH**:
- ✅ EPH extends with κ-based individual differences (Heins assumes homogeneous agents)
- ✅ EPH provides analytical β_c (Heins uses parameter sweep)
- 🤝 Shared FEP-based swarm modeling approach
- 📌 Heins has experimental validation (EPH needs this)

---

### Paper 2: ★★★ (Markov Blanket Theory)

**Authors**: Domenico Maisto, Davide Nuzzi, Giovanni Pezzulo
**Title**: What the flock knows that the birds do not: exploring the emergence of joint agency in multi-agent active inference
**Venue**: arXiv:2511.10835 (2025)
**URL**: https://arxiv.org/abs/2511.10835

**Summary**: Shows how group-level Markov Blankets emerge from interacting Active Inference agents, creating "flock knowledge" inaccessible to individuals.

**Relevance to EPH**:
- ✅ EPH uses "Broken Markov Blanket" (complementary to emergent MB)
- 🤝 Shared multi-agent Active Inference framework
- 📌 Synergistic information analysis applicable to EPH
- 💡 Reinterpret haze as information filtering mechanism

---

### Paper 3: ★★★ (Game Theory Integration)

**Authors**: Jaime Ruiz-Serra, Patrick Sweeney, Michael S. Harré
**Title**: Factorised Active Inference for Strategic Multi-Agent Interactions
**Venue**: AAMAS 2025
**URL**: https://arxiv.org/abs/2411.07362

**Summary**: Integrates Active Inference with game theory for strategic multi-agent planning with explicit beliefs about other agents' internal states.

**Relevance to EPH**:
- ✅ EPH focuses on continuous swarm dynamics (more general)
- 🤝 Both address collective optimization challenges
- 📌 Nash equilibrium analysis for 2:6:2 stability
- 💡 Game-theoretic interpretation of Leader-Follower interaction

---

### Paper 4: ★★★ (Phase Transition)

**Authors**: Chenchen Huang, Fangxu Ling, Eva Kanso
**Title**: Collective phase transitions in confined fish schools
**Venue**: PNAS, Vol. 121, No. 44, e2406293121 (2024)
**DOI**: 10.1073/pnas.2406293121
**URL**: https://www.pnas.org/doi/10.1073/pnas.2406293121

**Summary**: Models fish schools as self-propelled particles, discovering bistable states and spontaneous switching between schooling/milling in confined spaces.

**Relevance to EPH**:
- ✅ EPH provides analytical β_c (Kanso discovers numerically)
- 🤝 Shared phase transition focus
- 📌 Experimental validation methods
- 💡 Apply bifurcation analysis to EPH haze dynamics

---

### Paper 5: ★★ (Role Heterogeneity - Empirical)

**Authors**: Gal A. Kaminka, Yinon Douchan
**Title**: Heterogeneous foraging swarms can be better
**Venue**: Frontiers in Robotics and AI, Vol. 11 (2024)
**DOI**: 10.3389/frobt.2024.1426282
**URL**: https://www.frontiersin.org/journals/robotics-and-ai/articles/10.3389/frobt.2024.1426282/full

**Summary**: Demonstrates that robot swarms with heterogeneous behavioral roles outperform homogeneous swarms through distributed reinforcement learning.

**Relevance to EPH**:
- ✅ EPH derives 2:6:2 from variational principle (Kaminka discovers via learning)
- 🤝 Both emphasize role heterogeneity importance
- 📌 Real robot (Krembot) validation methods
- 💡 AEI reward function for EPH coordination evaluation

---

### Paper 6: ★★ (Role Heterogeneity - Evolutionary)

**Authors**: Fuda van Diggelen, Matteo De Carlo, Nicolas Cambier, Eliseo Ferrante, A.E. Eiben
**Title**: Emergence of specialized Collective Behaviors in Evolving Heterogeneous Swarms
**Venue**: PPSN 2024
**DOI**: 10.1007/978-3-031-70068-2_4
**URL**: https://arxiv.org/abs/2402.04763

**Summary**: Evolves robot swarms with phenotypic plasticity, showing emergence of specialized roles from shared genotype.

**Relevance to EPH**:
- ✅ EPH derives distribution theoretically (van Diggelen uses evolution)
- 🤝 Both explore role specialization mechanisms
- 📌 Phenotypic plasticity concept
- 💡 Interpret κ variation as phenotypic expression

---

### Paper 7: ★★ (Hierarchical Active Inference)

**Authors**: Kentaro Fujii, Shingo Murata
**Title**: Real-World Robot Control by Deep Active Inference With a Temporally Hierarchical World Model
**Venue**: IEEE RA-L (2025)
**URL**: https://arxiv.org/abs/2512.01924

**Summary**: Proposes temporally hierarchical world model for efficient robot control, achieving goal-directed and exploratory behavior switching.

**Relevance to EPH**:
- ✅ EPH focuses on multi-agent (Fujii on single robot)
- 🤝 Shared Active Inference control framework
- 📌 Temporal hierarchy implementation
- 💡 Extend f_θ with temporal hierarchy

---

### Paper 8: ★★ (MAS Coordination)

**Authors**: Lukas Beckenbauer, Johannes-Lucas Loewe, Ge Zheng, Alexandra Brintrup
**Title**: Orchestrator: Active Inference for Multi-Agent Systems in Long-Horizon Tasks
**Venue**: arXiv:2509.05651 (2025)
**URL**: https://arxiv.org/abs/2509.05651

**Summary**: Applies Active Inference to LLM-enhanced MAS for long-horizon task coordination with attention-based self-emergent coordination.

**Relevance to EPH**:
- ✅ EPH focuses on physical swarms (Orchestrator on LLM agents)
- 🤝 Shared Active Inference MAS framework
- 📌 Long-horizon coordination methods
- 💡 Information gain vs coordination cost trade-off analysis

---

## Research Positioning

```
FEP Theoretical Foundation (Friston)
    ↓
Paper 1: Heins et al. (Surprise Minimization → Swarm) ──────┐
Paper 2: Maisto et al. (Emergent Group MB) ─────────────────┤
Paper 3: Ruiz-Serra et al. (Game Theory Integration) ───────┤
    ↓                                                       │
EPH v2.0 (Haze + 2:6:2 + β_c) ←─────────────────────────────┘
    ↓
Paper 4: Kanso et al. (Phase Transition Validation)
Papers 5,6: Kaminka, van Diggelen (Role Heterogeneity Validation)
Papers 7,8: Fujii, Beckenbauer (Implementation Reference)
```

---

## EPH v2.0 Differentiation

| Aspect | Existing Work | EPH v2.0 |
|--------|--------------|----------|
| Individual differences | Homogeneous agents | κ distribution (2:6:2) |
| Phase transition | Numerical discovery | Analytical β_c ≈ 0.098 |
| Role distribution | Learning/Evolution result | Variational principle derivation |
| Markov Blanket | Emergent / Static | Broken (adaptive) |
| Convergence | Empirical | Proven (ρ ≈ 0.02) |

---

## Recommended Citations

### Essential (Must Cite)

1. **Heins et al. (2024)** - Direct theoretical predecessor
2. **Friston (2010)** - FEP foundation (if not already cited)

### Strongly Recommended

3. **Maisto et al. (2025)** - Markov Blanket in MAS
4. **Kanso et al. (2024)** - Phase transition in swarms
5. **Kaminka & Douchan (2024)** - Role heterogeneity empirical support

### Optional (For Depth)

6. **Ruiz-Serra et al. (2025)** - Game theory connection
7. **van Diggelen et al. (2024)** - Evolutionary role emergence
8. **Fujii & Murata (2025)** - Hierarchical Active Inference implementation

---

## BibTeX

```bibtex
@article{Heins2024,
  author = {Heins, Conor and Millidge, Beren and Da Costa, Lancelot and Mann, Richard P. and Friston, Karl J. and Couzin, Iain D.},
  title = {Collective behavior from surprise minimization},
  journal = {Proceedings of the National Academy of Sciences},
  volume = {121},
  number = {17},
  pages = {e2320239121},
  year = {2024},
  doi = {10.1073/pnas.2320239121}
}

@article{Maisto2025,
  author = {Maisto, Domenico and Nuzzi, Davide and Pezzulo, Giovanni},
  title = {What the flock knows that the birds do not: exploring the emergence of joint agency in multi-agent active inference},
  journal = {arXiv preprint arXiv:2511.10835},
  year = {2025}
}

@inproceedings{RuizSerra2025,
  author = {Ruiz-Serra, Jaime and Sweeney, Patrick and Harr{\'e}, Michael S.},
  title = {Factorised Active Inference for Strategic Multi-Agent Interactions},
  booktitle = {Proceedings of AAMAS 2025},
  year = {2025}
}

@article{Huang2024,
  author = {Huang, Chenchen and Ling, Fangxu and Kanso, Eva},
  title = {Collective phase transitions in confined fish schools},
  journal = {Proceedings of the National Academy of Sciences},
  volume = {121},
  number = {44},
  pages = {e2406293121},
  year = {2024},
  doi = {10.1073/pnas.2406293121}
}

@article{Kaminka2024,
  author = {Kaminka, Gal A. and Douchan, Yinon},
  title = {Heterogeneous foraging swarms can be better},
  journal = {Frontiers in Robotics and AI},
  volume = {11},
  year = {2024},
  doi = {10.3389/frobt.2024.1426282}
}

@inproceedings{vanDiggelen2024,
  author = {van Diggelen, Fuda and De Carlo, Matteo and Cambier, Nicolas and Ferrante, Eliseo and Eiben, A.E.},
  title = {Emergence of specialized Collective Behaviors in Evolving Heterogeneous Swarms},
  booktitle = {Parallel Problem Solving from Nature (PPSN 2024)},
  year = {2024},
  doi = {10.1007/978-3-031-70068-2_4}
}

@article{Fujii2025,
  author = {Fujii, Kentaro and Murata, Shingo},
  title = {Real-World Robot Control by Deep Active Inference With a Temporally Hierarchical World Model},
  journal = {IEEE Robotics and Automation Letters},
  year = {2025}
}

@article{Beckenbauer2025,
  author = {Beckenbauer, Lukas and Loewe, Johannes-Lucas and Zheng, Ge and Brintrup, Alexandra},
  title = {Orchestrator: Active Inference for Multi-Agent Systems in Long-Horizon Tasks},
  journal = {arXiv preprint arXiv:2509.05651},
  year = {2025}
}
```

---

*Survey completed: 2026-02-01*
*Surveyor: Claude (Related Work Survey Skill v2.1)*
*Papers reviewed: 8 (from 15+ candidates)*
