# Add a New React Component

Add a new React UI component to the WASM Quine Bootloader application following the conventions used in the existing `components/` directory.

## Instructions

1. Create the file `components/$ARGUMENTS.tsx`.
2. Use a **React functional component** with TypeScript. Export it as a named export.
3. Accept props via a typed `interface` defined in the same file (unless types already exist in `types.ts`).
4. Style exclusively with **Tailwind CSS** utility classes – no inline styles, no CSS modules.
5. Use `font-mono` for any text that represents system/kernel data.
6. Use `transition-*` and `duration-*` classes for smooth state-driven visual changes.
7. Wrap expensive computations in `useMemo`; wrap callbacks in `useCallback`.
8. Match the dark terminal aesthetic: backgrounds from `bg-black/*` or `bg-gray-900/*`; borders from `border-gray-800/*`; text from `text-gray-*` or accent colours (`text-cyan-*`, `text-green-*`, `text-yellow-*`, `text-red-*`, `text-purple-*`).
9. If the component reads `SystemEra` to vary its appearance, accept `era: SystemEra` as a prop and use a `switch` over `SystemEra` values (PRIMORDIAL, EXPANSION, COMPLEXITY, SINGULARITY).
10. Import any shared types from `../types` and any constants from `../constants`.
11. After creating the component, import and render it inside `App.tsx` in the appropriate layout position.

## Example skeleton

```tsx
import React from 'react';
import { SystemEra } from '../types';

interface MyComponentProps {
  era: SystemEra;
  value: number;
}

export const MyComponent: React.FC<MyComponentProps> = ({ era, value }) => {
  const accentColor = era === SystemEra.SINGULARITY ? 'text-red-400' : 'text-cyan-400';

  return (
    <div className="p-4 bg-black/30 border border-gray-800/50 font-mono">
      <div className={`text-xs uppercase tracking-wider ${accentColor}`}>
        {value}
      </div>
    </div>
  );
};
```
