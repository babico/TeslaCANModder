import FsdPackagePanel from './fsd/FsdPackagePanel';
import { createFsdState, deriveFsdState } from './fsd/package';

export const toolkitPackages = [
  {
    id: 'fsd',
    title: 'FSD',
    createState: createFsdState,
    deriveState: deriveFsdState,
    DashboardPanel: FsdPackagePanel,
  },
];
